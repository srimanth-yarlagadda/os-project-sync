#define nthreadsTotal 16

void arraySort_HL(void* ap);
void *arraySort(void* ap);

sem_t sorterFnMain;
sem_t printSignal;
pthread_mutex_t printMutex;

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
    struct request* next;
};

struct workarrays {
    int* workArray;
    int free;
    sem_t arraySemph;
    sem_t threadSemph[8];
    sem_t printSemaphore;
};

struct args {
    int* array;
    int array_offset;
    int array_size;
    sem_t* semph;
    int thid;
    int layerElements;
};