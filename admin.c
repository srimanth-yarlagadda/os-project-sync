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

#define max_string_size 512
#define mext 13
#define createthreads 16 // 8+4+2+1+1
#define nthreadsTotal 16
#define max_array_size 1024
#define detachThreads 1

FILE* fp;

#define masterDebug 0
pthread_t threads[nthreadsTotal];
int l0_thread_count = 8;

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

void testprinter(int* array, int n) {
    printf("\n\nPrinting off test printer\n\n");
    int i, j=4;
    printf("[");
    for (i = 0; i < n; i++) {
        if (j==0) {
            printf("\b] ["); j = 4;
        }
        printf("%d ", array[i]); 
        j--;
    }
    printf("\b]\n");
}

struct request* reqHead = NULL;
struct request* reqTail = NULL;

char holdbuffer[10240];

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
        // arrayList[i]->mergeStatus = (int*)malloc(4*sizeof(int));
        // for (j = 0; j < 4; j++) {
        //     arrayList[i]->mergeStatus[i] = 2;
        // }
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

void *sorter(void* inputptr) {
    printf("sorter called\n");
    // printf("Thread %lu created %d\n", (long unsigned int)pthread_self(), (int)pthread_self());
    
    while (1) {
        struct args* argin = (struct args*) inputptr;
        // printf("%p %p \n\n\n", argin, inputptr);
        sem_wait(&mpS[argin->thid]);
        if (argin->array == NULL) {
            // printf("IF sorter called\n");
            // printf(".");
            sem_post(&mpS[argin->thid]);
        }
        else {
            // printf("Else sorter called\n");
            int bef = -5, aft = -1;
            // printf("thid %d, inside else array %p\n", argin->thid, argin->array);
            sem_t* threadSp = getThreadSemph(argin->array, (argin->array_offset)/4);
            
            sem_getvalue(threadSp, &bef);
            sem_wait(threadSp);
            sem_getvalue(threadSp, &aft);
        
            if (masterDebug) {
                printf("Thread [%d] %lu before %d after %d for semaphore %p\n", 
                (argin->array_offset)/4, (long unsigned int)pthread_self(), bef, aft, threadSp);
            }

            int i, j, tmp, min; int n = argin->array_size;
            int* array = (int*) ((argin->array) + argin->array_offset);
            for (i=0; i<n-1; i++) {
                min = i;
                for (j=i+1; j<n; j++) {
                    if (array[j] < array[min]) {
                        min = j;
                    }
                }
                if (min!=i) {
                    tmp = array[i];
                    array[i] = array[min];
                    array[min] = tmp;
                }
            }

            if ((argin->thid)%2 == 1) {/*do nothing*/}
            else {
                mpA[(l0_thread_count) + ((argin->thid)/2)]->array = (argin->array);
                mpA[(l0_thread_count) + ((argin->thid)/2)]->array_offset = (argin->array_size)*2*(argin->thid)/2;
                mpA[(l0_thread_count) + ((argin->thid)/2)]->array_size = (argin->array_size)*2;
                sem_post(&mpS[(l0_thread_count) + ((argin->thid)/2)]);
            }
            sem_post(&mpStest[argin->thid]);
            argin->array = NULL;
            sem_post(threadSp);
            sem_post(&mpS[argin->thid]);
        }
    }
}



void *merger(void* inputptr) {
    // printf("MERGER CALLED\n");
    while (1) {
        struct args* argin = (struct args*) inputptr;
        // if (argin->thid==14) {
        //     printf("MERGER CALLED THID 4\n");
        //     sleep(0.5);
        // }
        sem_wait(&mpS[argin->thid]);
        if (argin->array == NULL) {
            sem_post(&mpS[argin->thid]);
        }
        else {
            int* array = (int*) ((argin->array) + argin->array_offset);
            int debug = 0;
            int sz = argin->array_size;
            // printf("iN else mereger\n");

            sem_t* threadSp1 = getThreadSemph(argin->array, ((argin->array_offset)*2/sz) + 0);
            sem_t* threadSp2 = getThreadSemph(argin->array, ((argin->array_offset)*2/sz) + 1);
            sem_wait(threadSp1); sem_wait(threadSp2);
            
            int i = (sz/2)-1, j = i, n = sz, tmp;
            int ii = 0, jj = sz/2; int k;
            while (i >= 0 && j >= 0) {
                if (array[ii] <= array[jj]) {
                    ii++;
                    i--;
                }
                else {
                    tmp = array[jj];
                    for (k = jj; k>ii; k--) {
                        array[k] = array[k-1];
                    }
                    array[ii] = tmp;
                    jj++; ii++;
                    j--;
                }
                if (debug) {
                    printf("First element in thread is : %d and pointer is : %p\n", array[0], array);
                    printf("Debug:\n");
                    printer(array, sz);
                    printf("ii is %d, jj is %d, [i,j]: %d %d\n\n", ii, jj, i, j);
                }
            }
            if ((argin->thid)%2 == 1) {/*do nothing*/}
            // else if ( argin->thid == 12 ) {
            //     mpA[14]->array = (argin->array);
            //     mpA[14]->array_offset = 0;
            //     mpA[14]->array_size = 32;
            //     sem_post(&mpS[14]);
            // }
            // else if (argin->thid == 14) {
            //     mpA[15]->array_size = (argin->array_size);
            //     mpA[15]->array = (argin->array);
            //     // sem_post(&mpS[15]);
            // }
            else {
                mpA[(l0_thread_count) + ((argin->thid)/2)]->array = (argin->array);
                mpA[(l0_thread_count) + ((argin->thid)/2)]->array_offset = (argin->array_size)*2* ((argin->thid)-(argin->array_size))/2;
                mpA[(l0_thread_count) + ((argin->thid)/2)]->array_size = (argin->array_size)*2;
                sem_post(&mpS[(l0_thread_count) + ((argin->thid)/2)]);
            }
            argin->array = NULL;
            
            sem_post(threadSp1); sem_post(threadSp2);
            sem_post(&mpS[argin->thid]);
            
            // pthread_mutex_unlock(&printMutex);
            // break;
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
    // printf("RECV %s\n", fname);
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
    req->filename = fname;
    req->input_array = input_buffer;
    req->input_length = input_length;

    return req;
}

void finalMerge(int* arr) {
    struct args* mergeparams = (struct args*) malloc(sizeof(struct args));
    mergeparams->array = arr;
    mergeparams->array_offset = 0;
    mergeparams->array_size = 32;
    merger(mergeparams);
    return;
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
    for ( i = 0; i < 4; i++) {
        mergeStatus[i] = 2;
        pthread_cond_init(&condWait[i], NULL);
    }
    for ( i = 0; i < 8; i++) {
        sem_init(&mpStest[i],0,1);
        sem_wait(&mpStest[i]);
    }
    return;
}

int initialINT = 0;

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
    initialINT -= 1;
    return;
}

int tar[12];

void initThreads() {
    int i;
    struct args* mergeparams;
    for (i = 0; i < 8; i++) {
        mergeparams = mpA[i];
        pthread_create(&threads[i], NULL, &sorter, (void*)mergeparams); 
        pthread_detach(threads[i]);
    }
    for (i = 8; i < 12; i++) {
        mergeparams = mpA[i];
        pthread_create(&threads[i], NULL, &merger, (void*)mergeparams);
        pthread_detach(threads[i]);
    }
    for (i = 12; i < 14; i++) {
        mergeparams = mpA[i];
        pthread_create(&threads[i], NULL, &merger, (void*)mergeparams);
        if (detachThreads) pthread_detach(threads[i]);
    }
        pthread_create(&threads[14], NULL, &merger, (void*)mergeparams);
        if (detachThreads) pthread_detach(threads[14]);

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
    int run = 5;
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

        char fname[200];
        
        while (run>0) {
            // fp = fopen("op.txt", "w");
            sleep(2);
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
                newReq->filename = fname;
                newReq->input_array = array;
                newReq->input_length = array_size;
                
                struct arraysortparams* para = (struct arraysortparams*) malloc(sizeof(struct arraysortparams));
                para->r = newReq;
                para->m = m;
                para->d = d;
                sem_wait(&sorterFnMain);
                arraySort_HL(para);


                // sleep(12);
                printf("acquire lock....");
                // sem_wait(&printSignal);
                // pthread_mutex_lock(&printMutex);
                printf("done\n");
                printf("Printing after signal:\n");
                sleep(12);
                printer(array, 32);
                // printf("\nReturned without waiting in HL\n");
                // putFreeArray(array);
                free(newReq);
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
        // snprintf(holdbuffer, sizeof(holdbuffer), "Hello ");
        // snprintf(holdbuffer, sizeof(holdbuffer), "World !!");
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