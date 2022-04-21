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


void printer(int* array, int n) {
    // sleep(3);
    int i, j=4;
    printf("[");
    fflush(stdout);
    for (i = 0; i < n; i++) {
        if (j==0) {
            printf("\b] ["); j = 4;
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
    printf("TP CAlled\n\n");
    while(1) {
        struct args* argin = (struct args*) printArgs;
        sem_wait(&mpS[15]);
        if (argin->array == NULL) {
            sem_post(&mpS[15]);
            // sleep(1.5);
        }
        else {
            int* arr = argin->array; int n = argin->array_size;
            int i, j=4;
            printf("Printing from TP: [");
            fflush(stdout);
            for (i = 0; i < n; i++) {
                if (j==0) {
                    printf("\b] ["); j = 4;
                    fflush(stdout);
                }
                printf("%d ", arr[i]); 
                fflush(stdout);
                j--;
            }
            printf("\b]\n");
            fflush(stdout);
            argin->array = NULL;
            sem_post(&mpS[15]);
        }
    }
}


struct request* reqHead = NULL;
struct request* reqTail = NULL;

void addReq(struct request* r)  {
    if (reqHead == NULL) {
        reqHead = r;
        reqTail = r;
    }
    else {
        reqTail->next = r;
        reqTail = r;
    }
}

struct request* getReq() {
    if (reqHead == NULL) return NULL;
    else {
        struct request* tmp = reqHead;
        reqHead = reqHead->next;
        return tmp;
    }
}

struct workarrays* arrayList[5];
void initArrays(int a) {
    int i = 0, j;
    for (i=0;i<a;i++) {
        arrayList[i] = (struct workarrays*) malloc(sizeof(struct workarrays));
        arrayList[i]->workArray = (int*)malloc(1024*sizeof(int));
        arrayList[i]->free = 1;
        // printf("INIT %p @ %d\n", arrayList[i]->workArray, i);
        sem_init(&(arrayList[i]->arraySemph), 0, 1);
        for (j = 0; j < 8; j++) {
            sem_init(&(arrayList[i]->threadSemph[j]), 0, 1);
            sem_wait(&(arrayList[i]->threadSemph[j]));
        }
        sem_init(&(arrayList[i]->printSemaphore), 0, 1);
    }
}
int* getFreeArray() {
    int i, slock_before, slock_after, j;
    for (i=0;i<5;i++) {
        if (arrayList[i]->free==1) {
            printf("Trying...%d...", i);
            sem_wait(&(arrayList[i]->arraySemph));
            arrayList[i]->free = 0;
            int* free_return = arrayList[i]->workArray;
            printf("Returning array %p @ %d !!\n", free_return, i);
            sem_wait(&(arrayList[i]->printSemaphore));
            return free_return;
        }
        // else return NULL;
    }
    return NULL;
}

void putFreeArray(int* arr) {
    int i;
    for (i=0;i<5;i++) {
        if (arrayList[i]->workArray == arr) {
            arrayList[i]->free = 1;
            sem_post(&(arrayList[i]->printSemaphore));
            sem_post(&(arrayList[i]->arraySemph));
            printf("Array %p @ %d put successful\n", arr, i);
        }
    }
    return;
}

sem_t* getThreadSemph(int* array, int offset) {
    int i, j;
    for (i=0;i<5;i++) {
        if (arrayList[i]->workArray==array) {
            for (j = 0; j < 8; j++) {
                return &(arrayList[i]->threadSemph[offset]);
            }
        }
    }
}

sem_t* getPrintSemaphore(int* array) {
    int i;
    for (i=0;i<5;i++) {
        if (arrayList[i]->workArray==array) {
            return &(arrayList[i]->printSemaphore);
        }
    }
}



int createServer() {
    int i;
    // following code is taken from slides used in lecture
    int soc = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    bzero ((char *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(1038);
    // printf("Socket in admin.c is: %d\n", soc);

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
    // printf("Socket in admin.c is: %d\n", soc);
    int listen_status = listen(soc, 100);
    if (listen_status == 0) {
        // printf("Listening...\n");
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
    // printf("RECV NAME:  %s\n", fname);
    recv(acceptance_fd, &input_length, sizeof(int), 0);
    // printf("Array %s Size is: %d\n", fname, input_length);
    
    input_buffer = (int*) malloc(sizeof(int)*input_length);
    int tmp;
    for (i = 0; i < input_length; i++) {
        recv(acceptance_fd, &tmp, sizeof(int), 0);
        input_buffer[i] = tmp;
     }
    // printf("Printing received data at socket: \n");
    // printer(input_buffer, input_length);
    // printf("\n");
    // close(soc);
    close(acceptance_fd);
    struct request* req = (struct request*) malloc(sizeof(struct request));
    *(req->filename) = *fname;
    // printf("RECV NAME:  %s\n", req->filename);
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
        if (r!=0) printf("ERROR IN THREAD CREATION %d !\n", r);
    }
    for (i = 8; i < 15; i++) {
        mergeparams = mpA[i];
        r = pthread_create(&threads[i], NULL, &merger, (void*)mergeparams);
        if (r!=0) printf("ERROR IN THREAD CREATION %d !\n", r);
    }
    for (i = 0; i < 15; i++) {
        pthread_detach(threads[i]);
    }
    // for (i = 12; i < 14; i++) {
        // mergeparams = mpA[i];
        // pthread_create(&threads[i], NULL, &merger, (void*)mergeparams);
        // if (detachThreads) pthread_detach(threads[i]);
    // }
    // pthread_create(&threads[14], NULL, &merger, (void*)mergeparams);
    // if (detachThreads) pthread_detach(threads[14]);

        // pthread_create(&threads[15], NULL, &threadPrinter, (void*)mergeparams);
        // if (detachThreads) pthread_detach(threads[15]);
    return;
}

int main() {
    // system("clear");
    int m=8,a=5,q=5,d=0;
    // printf("Enter number of parallel threads: \n");
    // scanf("%d",&m);
    // printf("Enter number of arrays: \n");
    // scanf("%d",&a);
    // printf("Enter queue size: \n");
    // scanf("%d",&q);
    // printf("Enable debug? [1 for yes / 0 for no]: \n");
    // scanf("%d",&d);
    printf("Starting with following parameters: M %d, A %d, D %d, Q %d\n", m,a,d,q);
    int p, to_cal[2], from_cal[2];
    if (pipe(to_cal) == -1) {printf("Send pipe creation error !");};
    if (pipe(from_cal) == -1) {printf("Receive pipe creation error !");};
    int run = 10;
    p = fork();
    
    if (p == 0) {

        int array_size, i, tmp, thread_count = m, read_success = 0;
        int *array;
        initArrays(a);
        sem_init(&sorterFnMain, 0, 1);
        sem_init(&printSignal, 0, 1);
        sem_wait(&printSignal);
       
        pthread_mutex_init(&printMutex, NULL);
        pthread_mutex_lock(&printMutex);
        initMPA();
        initThreads();

        struct args* mergeparams;

        char fname[200];
        
        while (run>0) {
            printf("\n");
            array = getFreeArray();
            if (array != NULL) {
                read(to_cal[0], &fname, sizeof(fname));
                write(from_cal[1], &read_success, sizeof(read_success));
                // printf("Request \"%s\"\n\t", fname);
                read(to_cal[0], (&array_size), sizeof(int));
                write(from_cal[1], &read_success, sizeof(read_success));
                
                // array = malloc(array_size*sizeof(int));
                for (i=0; i<array_size;i++) {
                    read(to_cal[0], &(array[i]), sizeof(int));
                    write(from_cal[1], &read_success, sizeof(read_success));
                }
                
                printf("RECD:\n"); printer(array, 32);

                struct request* newReq = (struct request*) malloc(sizeof(struct request));
                *(newReq->filename) = *fname;
                newReq->input_array = array;
                newReq->input_length = array_size;
                
                struct arraysortparams* para = (struct arraysortparams*) malloc(sizeof(struct arraysortparams));
                para->r = newReq;
                para->m = m;
                para->d = d;
                // sem_wait(&sorterFnMain);
                
                // arraySort_HL(para);


                int inc = 0;    
                int i;
                for ( i = 0; i < 8; i++ ){
                    mergeparams = mpA[i];
                    mergeparams->array_offset = inc;
                    mergeparams->array_size = 4;
                    mergeparams->array = array + inc;
                    // printf("Semp ptr %p\n", &mpS[i]);
                    sem_post(&mpS[i]);
                    inc += 4;
                }


                sem_wait(getPrintSemaphore(array));    
                printf("Printing Result of Request %s:\n", newReq->filename);
                // sleep(20);
                printer(array, 32);
            
                putFreeArray(array);
                free(newReq);
                clearMergeParams();
                // fname = "";
                run--;
            }
            else {
                printf("Got no free array!\n"); sleep(2);
            }
        }

    }
    else {

        close(to_cal[0]);
        int socket = createServer();
        struct request* currentreq;
        int* arr;
        int i, read_status = -1;

        while (run>0) {
            currentreq = receive(socket);
            arr = currentreq->input_array;
        
            // printf("Sending %s from parent to child\n", currentreq->filename);
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
            run--;
        }
        // printf("%s", holdbuffer);
        // sleep(20);
        kill(p, SIGKILL);
        close(socket);

    }

    return 0;
}