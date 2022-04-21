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

void *sorter(void* inputptr) {
    // printf("sorter called\n");
    // printf("Thread %lu created %d\n", (long unsigned int)pthread_self(), (int)pthread_self());
    
    while (1) {
        struct args* argin = (struct args*) inputptr;
        // printf("%p %p \n\n\n", argin, inputptr);
        sem_wait(&mpS[argin->thid]);
        if (argin->array == NULL) {
            sem_post(&mpS[argin->thid]);
        }
        else {
            // printf("Else sorter called\n");
            int bef = -5, aft = -1;
            // printf("thid %d, inside else array %p\n", argin->thid, argin->array);
            sem_t* threadSp = getThreadSemph(argin->array, (argin->array_offset)/4);
            
            // sem_getvalue(threadSp, &bef);
            // sem_wait(threadSp);
            // sem_getvalue(threadSp, &aft);
        
            if (masterDebug) {
                printf("Thread [%d] %lu before %d after %d for semaphore %p\n", 
                (argin->array_offset)/4, (long unsigned int)pthread_self(), bef, aft, threadSp);
            }

            int i, j, tmp, min; int n = argin->array_size;
            int* array = (int*) ((argin->array));
            
            // printf("%d %p %d \n", argin->thid, argin->array, argin->array_offset);
            
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

            sem_wait(&mStatS[ (argin->thid) ]);
            mergeStatus[ (argin->thid) ] = mergeStatus[ (argin->thid) ] + 1;
            sem_post(&mStatS[ (argin->thid) ]);
            
            if ((argin->thid)%2 == 1) {/*do nothing*/}
            else if ( (argin->thid) >= 8) {/*do nothing*/}
            else {
                mpA[(l0_thread_count) + ((argin->thid)/2)]->array = (argin->array);
                mpA[(l0_thread_count) + ((argin->thid)/2)]->array_offset = (argin->array_size)*2*(argin->thid)/2;
                mpA[(l0_thread_count) + ((argin->thid)/2)]->array_size = (argin->array_size)*2;
                
                if (0) {
                    printf("SIG %d from %d (Sorter) - ar %p, offset %d, size %d\n", 
                    (l0_thread_count) + ((argin->thid)/2), argin->thid, argin->array, (argin->array_size)*2*(argin->thid)/2, (argin->array_size)*2 );
                }
                
                sem_post(&mpS[ (l0_thread_count) + ((argin->thid)/2)]);
            }
            sem_post(&mpStest[argin->thid]);
            argin->array = NULL;
            // sem_post(threadSp);
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
            int* array = (int*) ((argin->array));
            int debug = 0, test = -1;
            int sz = argin->array_size;
            int times = 8/argin->layerElements;
            // printf("iN else mereger\n");
            int it, base;

            if (argin->thid >=8 && argin->thid < 12) {
                base = (argin->thid - 8 ) * 2;
            }
            else if (argin->thid >= 12 && argin->thid < 14) {
                base = argin->thid;
                base = base-12;
                base = base * 4;
            }
            else if (argin->thid == 14) {
                base = 0;
            }
            
            
            while (1) {
                test = 0;
                for (it = 0; it < times; it++) {
                    sem_wait(&mStatS[ base+it  ]);
                    test += (mergeStatus[ base+it  ]);
                    sem_post(&mStatS[ base+it  ]);
                }

                if (test == 2) break;
                if (test == 8) break;
                if (test == 24) break;
                if (test == 32) break;
                sleep(1);
            }

            sem_t* threadSp1 = getThreadSemph(argin->array, ((argin->array_offset)*2/sz) + 0);
            sem_t* threadSp2 = getThreadSemph(argin->array, ((argin->array_offset)*2/sz) + 1);
            // sem_wait(threadSp1); sem_wait(threadSp2);
            if (0) {
                if (argin->thid == 14) printf("=====>>>");
                printf("%d %p %d \n", argin->thid, argin->array, argin->array_offset);
            }
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

            for (it = 0; it < times; it++) {
                sem_wait(&mStatS[ base+it ]);
                if (0) printf("updating %d from %d\n", base+it, argin->thid);
                mergeStatus[ base+it  ] = mergeStatus[ base+it  ] + 1;
                sem_post(&mStatS[ base+it  ]);
            }
            if (argin->thid == 14) {
                /*Give Print Signal*/
                sem_post(getPrintSemaphore(argin->array));
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
                if (0) {
                    printf("SIG %d from %d (Merger) - ar %p, offset %d, size %d\n", 
                    (l0_thread_count) + ((argin->thid)/2), argin->thid, argin->array, (argin->array_size)*2*(argin->thid)/2, (argin->array_size)*2 );
                }
                sem_post(&mpS[(l0_thread_count) + ((argin->thid)/2)]);
            }
            argin->array = NULL;
            
            // sem_post(threadSp1); sem_post(threadSp2);
            sem_post(&mpS[argin->thid]);
            
            // pthread_mutex_unlock(&printMutex);
            // break;
        }
    }
}

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