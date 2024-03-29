#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "definitions.h"

int getReqID() { return numRequests++; }

void ctrlc_handle() {
    killval = 1;
}

void printer(int* array, int n) {
    int i, j=10;
    printf("[");
    fflush(stdout);
    for (i = 0; i < n; i++) {
        if (j==0) {
            printf("\b] ["); j = 10;
            fflush(stdout);
        }
        printf("%d ", array[i]); 
        fflush(stdout);
        j--;
    }
    printf("\b]\n");
    fflush(stdout);
}

void *threadPrinter(void *printArgs) {
    while(1) {
        struct args* argin = (struct args*) printArgs;
        sem_wait(&mpS[15]);
        if (argin->array == NULL) {
            sem_post(&mpS[15]);
        }
        else {
            int* arr = argin->array; int n = argin->array_size;
            int i, jj = 10, j=jj;
            pthread_mutex_lock(&printMutex);
            printf("Result :: RequestID = %d, Request Name: %s, N = %2d\n[", (argin->thisRequest)->reqID, 
            (argin->thisRequest)->filename, argin->array_size_absolute);
            fflush(stdout);
            for (i = 0; i < argin->array_size_absolute; i++) {
                if (j==0) {
                    printf("\b]\n["); j = jj;
                    fflush(stdout);
                }
                printf("%d ", arr[i]); 
                fflush(stdout);
                j--;
            }
            printf("\b]\n");
            fflush(stdout);
            pthread_mutex_unlock(&printMutex);
            mpA[16]->array = argin->array;
            sem_post(&mpS[16]);
            argin->array = NULL;
            sem_post(&mpS[15]);
        }
    }
}


struct request* reqHead = NULL;
struct request* reqTail = NULL;

void printReqQ() {
    struct request* tmp;
    tmp = reqHead;
    while(tmp!=NULL) {
        printf("%p ", tmp);
        tmp=tmp->next;
    }
    printf("\n");
    return;
}

void insertReq(struct request* r)  {
    if (numRequestsInQ >= Q) {
        pthread_mutex_lock(&printMutex);
        printf(" ===== Queue at Capacity. Ignoring this [\" %s \"] request ===== \n", r->filename);
        pthread_mutex_unlock(&printMutex);
        free(r);
        return;
    }
    if (reqHead == NULL) {
        reqHead = r;
        reqTail = r;
        numRequestsInQ += 1;
    }
    else {
        reqTail->next = r;
        reqTail = r;
        numRequestsInQ += 1;
    }
    return;
}

struct request* getReq() {
    struct request* tmp;
    if (reqHead == NULL) {
        return NULL;
    }
    else {
        tmp = reqHead;
        reqHead = reqHead->next;
        numRequestsInQ -= 1;
        return tmp;
    }
}

struct workarrays* arrayList[20];

void initArrays(int a) {
    int i = 0, j;
    for (i=0;i<a;i++) {
        arrayList[i] = (struct workarrays*) malloc(sizeof(struct workarrays));
        arrayList[i]->workArray = (int*)malloc(1024*sizeof(int));
        arrayList[i]->free = 1;
        arrayList[i]->thisRequest = NULL;
        sem_init(&(arrayList[i]->arraySemph), 0, 1);
        for (j = 0; j < 8; j++) {
            sem_init(&(arrayList[i]->threadSemph[j]), 0, 1);
            sem_wait(&(arrayList[i]->threadSemph[j]));
        }
        sem_init(&(arrayList[i]->printSemaphore), 0, 1);
    }
}
struct workarrays* getFreeArray() {
    int i, slock_before, slock_after, j;
    for (i=0;i<a;i++) {
        if (arrayList[i]->free==1) {
            pthread_mutex_lock(&printMutex); 
                sem_wait(&(arrayList[i]->arraySemph));
                arrayList[i]->free = 0;
                struct workarrays* free_return = arrayList[i];
            pthread_mutex_unlock(&printMutex);
            sem_wait(&(arrayList[i]->printSemaphore));
            return free_return;
        }
    }
    return NULL;
}

void putFreeArray(int* arr) {
    int i;
    for (i=0;i<a;i++) {
        if (arrayList[i]->workArray == arr) {
            arrayList[i]->free = 1;
            sem_post(&(arrayList[i]->printSemaphore));
            sem_post(&(arrayList[i]->arraySemph));
            pthread_mutex_lock(&printMutex); 
                printf("Array %p @ %d put successful\n", arr, i);
            pthread_mutex_unlock(&printMutex);
        }
    }
    return;
}

void *putFreeArrayThread(void* inputptr) {
    while(1) {
        int* arr = (int*) inputptr;
        sem_wait(&mpS[16]);
        if (arr == NULL) {
            sem_post(&mpS[16]);
        }
        else {
            int i;
            for (i=0;i<a;i++) {
                if (arrayList[i]->workArray == arr) {
                    arrayList[i]->free = 1;
                    free(arrayList[i]->thisRequest);
                    arrayList[i]->thisRequest = NULL;
                    sem_post(&(arrayList[i]->printSemaphore));
                    sem_post(&(arrayList[i]->arraySemph));
                    if (0) {
                        pthread_mutex_lock(&printMutex); 
                            printf("Array %p @ %d put successful ON THREAD\n", arr, i);
                        pthread_mutex_unlock(&printMutex);
                    }
                }
            }
            arr = NULL;
            sem_post(&mpS[16]);
            if (fs) sleep(3);
        }
    }
}

sem_t* getThreadSemph(int* array, int offset) {
    int i, j;
    for (i=0;i<a;i++) {
        if (arrayList[i]->workArray==array) {
            for (j = 0; j < 8; j++) {
                return &(arrayList[i]->threadSemph[offset]);
            }
        }
    }
}

sem_t* getPrintSemaphore(int* array) {
    int i;
    for (i=0;i<a;i++) {
        if (arrayList[i]->workArray==array) {
            return &(arrayList[i]->printSemaphore);
        }
    }
}


int createServer() {
    int i;
    int soc = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    bzero ((char *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(1038);

    int bind_ret = bind(soc, (struct sockaddr *) &addr, sizeof(addr));
    if (bind_ret < 0) {printf("======[ Error while binding ]======\n");}
    else {
        // printf("Binding Success !!\n");
        }
    return soc;
}

struct request* receive(int soc) {
    int i;
    struct sockaddr_in addr;
    bzero ((char *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(1038);
    int listen_status = listen(soc, 100);
    if (listen_status == 0) {
        fflush(stdout);
    }
    else {
        printf("Listen Error !!\n");
    }
    
    int acceptance_fd = accept(soc, NULL, NULL);
    if (acceptance_fd < 0) {
        printf("Error Accepting!\n");
    }
    else {
        // printf("Connection Accepted !!\n");
    }

    char fname[200]; int input_length; int* input_buffer;
    recv(acceptance_fd, &fname, sizeof(fname), 0);
    recv(acceptance_fd, &input_length, sizeof(int), 0);
    
    input_buffer = (int*) malloc(sizeof(int)*input_length);
    int tmp;
    for (i = 0; i < input_length; i++) {
        recv(acceptance_fd, &tmp, sizeof(int), 0);
        input_buffer[i] = tmp;
     }
    close(acceptance_fd);
    struct request* req = (struct request*) malloc(sizeof(struct request));
    *(req->filename) = *fname;
    req->input_array = input_buffer;
    req->input_length = input_length;

    return req;
}


struct arraysortparams {
    struct request* r;
    int m; int d;
};

void initMPA() {
    int i;
    struct args *mergeparams;
    for ( i = 0; i < nthreadsTotal; i++) {
        mergeparams = (struct args*) malloc(sizeof(struct args));
        mergeparams->thid = i;
        sem_init(&mpS[i], 0, 1);
        sem_wait(&mpS[i]);
        mpA[i] = mergeparams;
    }
    for ( i = 0; i < 8; i++) {
        mergeStatus[i] = 0;
        pthread_cond_init(&condWait[i], NULL);
    }
    for ( i = 0; i < 8; i++) {
        sem_init(&mpStest[i],0,1);
        sem_wait(&mpStest[i]);
    }
    for ( i = 0; i < 8; i++) {
        sem_init(&mStatS[i],0,1);
    }
    for (i=0; i<8; i++) {
        mpA[i]->layerElements = 8;
    }
    for (i=8; i<12; i++) {
        mpA[i]->layerElements = 4;
    }
    for (i=12; i<14; i++) {
        mpA[i]->layerElements = 2;
    }
    mpA[14]->layerElements = 1;
    return;
}

void *arraySort(void* ap) {
    struct arraysortparams* params = (struct arraysortparams*) ap;
    int m = params->m;
    int d = params->d;
    struct request* newrequest = params->r;
    int i, thread_count = m;
    
    struct args *mergeparams;
    int* arr = newrequest->input_array;
    int sort_segment = newrequest->input_length / m;
    
    int initstatus, semret = -5;

    int inc = 0;    
    for ( i = 0; i < 8; i++ ){
        mergeparams = mpA[i];
        mergeparams->array_offset = inc;
        mergeparams->array_size = sort_segment;
        mergeparams->array = arr;
        sem_post(&mpS[i]);
        inc += (mergeparams->array_size);
    }
    return;
}

void clearMergeParams() {
    int i;
    struct args *mergeparams;
    for ( i = 0; i < nthreadsTotal; i++) {
        mergeparams = mpA[i];
        sem_wait(&mpS[i]);
    }
    for ( i = 0; i < 8; i++) {
        mergeStatus[i] = 0;
    }
    return;
}


void initThreads() {
    int i,r;
    struct args* mergeparams;
    for (i = 0; i < 8; i++) {
        mergeparams = mpA[i];
        r = pthread_create(&threads[i], NULL, &sorter, (void*)mergeparams); 
        if (r!=0) printf("ERROR IN THREAD [%2d] CREATION - %d !\n", i, r);
    }
    for (i = 8; i < 15; i++) {
        mergeparams = mpA[i];
        r = pthread_create(&threads[i], NULL, &merger, (void*)mergeparams);
        if (r!=0) printf("ERROR IN THREAD [%2d] CREATION - %d !\n", i, r);
    }
    for (i = 0; i < 15; i++) {
        pthread_detach(threads[i]);
    }

    mergeparams = mpA[15];
    r = pthread_create(&threads[15], NULL, &threadPrinter, (void*)mergeparams);
    if (r!=0) printf("ERROR IN THREAD [15] CREATION - %d !\n", r);
    pthread_detach(threads[15]);

    mergeparams = mpA[16];
    r = pthread_create(&threads[16], NULL, &putFreeArrayThread, (void*)mergeparams);
    if (r!=0) printf("ERROR IN THREAD [16] CREATION - %d !\n", r);
    pthread_detach(threads[15]);
    return;
}

void* receiver(void* sock) {
    struct request *r;
    int s = *(int*)sock;
    while(1) {
        r = receive(s);
        pthread_mutex_lock(&queueMutex);
        insertReq(r);
        pthread_mutex_unlock(&queueMutex);
    }
}

int main() {
    // system("clear");
    int m=8,q=5,d=0;
    a=20;

    printf("Enter queue size: \n");
    scanf("%d",&Q);
    
    printf("Starting program...\n");
    numRequestsInQ = 0;
    int p, to_cal[2], from_cal[2];
    numRequests = 0;
    if (pipe(to_cal) == -1) {printf("Send pipe creation error !");};
    if (pipe(from_cal) == -1) {printf("Receive pipe creation error !");};
    int run = 500;
    p = fork();
    
    if (p == 0) {

        int array_size, i, tmp, thread_count = m, read_success = 0;
        int *array;
        struct workarrays* wa;
        initArrays(a);
        sem_init(&sorterFnMain, 0, 1);
        sem_init(&printSignal, 0, 1);
        sem_wait(&printSignal);
       
        pthread_mutex_init(&printMutex, NULL);
        initMPA();
        initThreads();

        struct args* mergeparams;

        char fname[200];
        
        while (run>0) {
            printf("\n");
            
            wa = getFreeArray();
            array = wa->workArray;
            if (array != NULL) {
                read(to_cal[0], &fname, sizeof(fname));
                write(from_cal[1], &read_success, sizeof(read_success));
                
                read(to_cal[0], (&array_size), sizeof(int));
                write(from_cal[1], &read_success, sizeof(read_success));
                
                
                for (i=0; i<array_size;i++) {
                    read(to_cal[0], &(array[i]), sizeof(int));
                    write(from_cal[1], &read_success, sizeof(read_success));
                }

                struct request* newReq = (struct request*) malloc(sizeof(struct request));
                memset(newReq->filename, 0, 200);
                *(newReq->filename) = *fname;
                newReq->input_array = array;
                newReq->input_length = array_size;
                newReq->reqID = getReqID();

                pthread_mutex_lock(&printMutex);
                printf("Received filename %s, Request ID: %d\n", fname, newReq->reqID); 
                printer(array, newReq->input_length);
                pthread_mutex_unlock(&printMutex);
                
                struct arraysortparams* para = (struct arraysortparams*) malloc(sizeof(struct arraysortparams));
                para->r = newReq;
                para->m = m;
                para->d = d;

                int inc = 0;    
                int i;
                for ( i = 0; i < 8; i++ ){
                    mergeparams = mpA[i];
                    mergeparams->array_offset = inc;
                    mergeparams->array_size = (newReq->input_length)/8;
                    mergeparams->array_size_absolute = newReq->input_length;
                    mergeparams->array = array + inc;
                    mergeparams->thisRequest = newReq;
                    // printf("Semp ptr %p\n", &mpS[i]);
                    sem_post(&mpS[i]);
                    inc += (newReq->input_length)/8;
                }


                sem_wait(getPrintSemaphore(array));
                
                clearMergeParams();
                memset(fname, 0, 200);
                run--;
            }
            else {
                pthread_mutex_lock(&printMutex); printf("Got no free array!\n"); pthread_mutex_unlock(&printMutex); sleep(2);
            }
        }

    }
    else {

        close(to_cal[0]);
        int socket = createServer();
        struct request* currentreq;
        int* arr;
        int i, read_status = -1, r;
        killval = 0;

        pthread_t rcv;
        r = pthread_create(&rcv, NULL, &receiver, (void*)&socket);
        if (r!=0) printf("ERROR IN THREAD [%2d] CREATION - %d !\n", -1, r);
        pthread_detach(rcv);

        pthread_mutex_init(&queueMutex, NULL);

        while (run>0 && (killval != 1)) {
            currentreq = NULL;
        
            while (currentreq == NULL) {
                pthread_mutex_lock(&queueMutex);
                currentreq = getReq();
                pthread_mutex_unlock(&queueMutex);
                sleep(1); /* to reduce the frequency of busy waiting */
            }
            arr = currentreq->input_array;
        
            write(to_cal[1], currentreq->filename, 200);
            read(from_cal[0], &read_status, sizeof(int));

            write(to_cal[1], &(currentreq->input_length), sizeof(int));
            read(from_cal[0], &read_status, sizeof(int));

            for (i=0; i < (currentreq->input_length); i++) {
                read_status = -1;
                write(to_cal[1], &(arr[i]), sizeof(int));
                read(from_cal[0], &read_status, sizeof(int));
                if (read_status == -1) {printf("Read failure from child !!");}
            }
            free(arr);
            free(currentreq);
            run--;
        }

        kill(p, SIGKILL);
        close(socket);

    }

    return 0;
}