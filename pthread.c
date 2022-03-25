// ***********************************************************************
// Thread function increments global int counter, but done incorrectly
// ***********************************************************************

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define Max 10
#define Limit 1000000

int counter = 0;

// function that each thread will execute
void *increment (void *arg)
{ int j, id = (intptr_t) arg;

  printf ("Thread %d  started!\n", id);
  for (j=0; j<Limit; j++) { counter++; }
  printf ("Thread %d  is ending!\n", id);
}

void main()
{ int i, numT;

  printf ("Enter number of threads to create: (<= 10)");
  scanf ("%d", &numT);
  pthread_t arrayofthreads[numT];

  // Create numT of identical threads 
  for ( i = 0; i < numT; i++ ){
    // CODE NEEDED:
    // Add code here to create thread to run function "increment"
    // Pass "i" to the thread function
    // "i" is treated as our own thread id for easy identification

    intptr_t* ptr = malloc(sizeof(intptr_t));
    *ptr = i;
    pthread_create(&arrayofthreads[i], NULL, &increment, ptr);
    counter = 0;
  }

  // CODE NEEDED: wait for all the threads to finish
  for (i = 0; i < numT; i++) {
    pthread_join(arrayofthreads[i], NULL);
  }

  // Print the final value of counter
  printf ("\nFinal value of counter: %d (should be %d)\n",
          counter, numT * Limit);
}

