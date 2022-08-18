#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include "zemaphore.h"

#define NUM_THREADS 5
#define NUM_ITER 5

zem_t zema[NUM_THREADS];

void *justprint(void *data)
{
  int thread_id = *((int *)data);
  int next_id = (thread_id+1)%NUM_THREADS;

  for(int i=0; i < NUM_ITER; i++)
    {
      zem_down(&zema[thread_id]);
      printf("This is thread %d\n", thread_id);
      zem_up(&zema[next_id]);
    }
  return 0;
}

int main(int argc, char *argv[])
{
  zem_init(&zema[0], 1); // since, thread 0 has to begin...
  for(int i = 1; i<NUM_THREADS; i++){
    zem_init(&zema[i], 0); //used for ordering the threads
  }

  pthread_t mythreads[NUM_THREADS];
  int mythread_id[NUM_THREADS];

  
  for(int i =0; i < NUM_THREADS; i++)
    {
      mythread_id[i] = i;
      pthread_create(&mythreads[i], NULL, justprint, (void *)&mythread_id[i]);
    }
  
  for(int i =0; i < NUM_THREADS; i++)
    {
      pthread_join(mythreads[i], NULL);
    }
  
  return 0;
}
