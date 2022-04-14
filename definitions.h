#define nthreadsTotal 16

void arraySort_HL(void* ap);
void *arraySort(void* ap);

sem_t sorterFnMain;

struct args* mpA[nthreadsTotal];
sem_t        mpS[nthreadsTotal];
int          mergeStatus[4];
pthread_cond_t condWait[4]; 

struct request {
    char* filename;
    int* input_array;
    int input_length;
    struct request* next;
};

struct workarrays {
    int* workArray;
    int free;
    sem_t arraySemph;
    sem_t threadSemph[8];
    // sem_t 
};

struct args {
    int* array;
    int array_offset;
    int array_size;
    sem_t* semph;
    int thid;
};