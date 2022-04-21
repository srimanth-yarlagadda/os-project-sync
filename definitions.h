#define nthreadsTotal 17
#define max_string_size 512
#define mext 13
#define max_array_size 1024
#define detachThreads 1
#define masterDebug 0
#define l0_thread_count 8
#define forcesleep 0 /* make this 1 to test usage of multiple arrays - forces return array thread to delay */

int numRequests;
// int run;
int killval;

void arraySort_HL(void* ap);
void *arraySort(void* ap);
void *sorter(void* inputptr);
void *merger(void* inputptr);
sem_t* getThreadSemph(int* array, int offset);
sem_t* getPrintSemaphore(int* array);

pthread_t threads[nthreadsTotal];
sem_t sorterFnMain;
sem_t printSignal;
pthread_mutex_t printMutex, queueMutex;

struct args* mpA[nthreadsTotal];
sem_t        mpS[nthreadsTotal];
sem_t        mpStest[8];
int          mergeStatus[8];
sem_t        mStatS[8];
pthread_cond_t condWait[4];
pthread_mutex_t mergeStatusMutex[4];

struct request {
    char filename[200];
    int* input_array;
    int input_length;
    int reqID;
    struct request* next;
};

struct workarrays {
    int* workArray;
    int free;
    sem_t arraySemph;
    sem_t threadSemph[8];
    sem_t printSemaphore;
    struct request* thisRequest;
};

struct args {
    int* array;
    int array_offset;
    int array_size;
    int array_size_absolute;
    sem_t* semph;
    int thid;
    int layerElements;
    struct request* thisRequest;
};