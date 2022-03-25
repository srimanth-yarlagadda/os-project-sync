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
{ int j, id = *(int*) arg;

  printf ("Thread %d  started!\n", id);
  for (j=0; j<Limit; j++) { counter++; }
  printf ("Thread %d  is ending!\n", id);
}

void main()
{ int i, numT;

  printf ("Enter number of threads to create: (<= 10)");
  scanf ("%d", &numT);
  pthread_t threads[numT];

  // Create numT of identical threads 
  for ( i = 0; i < numT; i++ ){
	
	intptr_t *ptr;
	ptr = malloc(sizeof(intptr_t));
	*ptr = i;
	pthread_create(&threads[i], NULL, &increment, ptr);
	counter = 0;
	
    // CODE NEEDED:
    // Add code here to create thread to run function "increment"
    // Pass "i" to the thread function
    // "i" is treated as our own thread id for easy identification
  }

  // CODE NEEDED: wait for all the threads to finish
  
  for(i = 0; i < numT; i++)
  {
	pthread_join(threads[i], NULL);
  }
  // Print the final value of counter
  printf ("\nFinal value of counter: %d (should be %d)\n",
          counter, numT * Limit);
}