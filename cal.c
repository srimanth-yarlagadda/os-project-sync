#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "definitions.h"

void arraySort_HL(void* ap) {
    pthread_t th;
    pthread_create(&th, NULL, &arraySort, ap);
    pthread_detach(th);
    // printf("Final: \n");
    // printf("Ending MASTER thread\n\n");
    // printer( (((struct args*)ap)->array), 32);
    // pthread_join(th, NULL);
    return;
};