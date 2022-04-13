void arraySort_HL(void* ap);
void *arraySort(void* ap);

sem_t sorterFnMain;

struct args* mpA[8];
sem_t        mpS[8];

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
};

struct args {
    int* array;
    int array_offset;
    int array_size;
    sem_t* semph;
    int* sortstatus;
    int thid;
};