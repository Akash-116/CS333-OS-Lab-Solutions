#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

int item_to_produce, curr_buf_size;
int total_items, max_buf_size, num_workers, num_masters;

int *buffer;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t write_available = PTHREAD_COND_INITIALIZER;
pthread_cond_t read_available = PTHREAD_COND_INITIALIZER;

void print_produced(int num, int master) {

  printf("Produced %d by master %d\n", num, master);
}

void print_consumed(int num, int worker) {

  printf("Consumed %d by worker %d\n", num, worker);
  
}

// write function to be run by worker threads
// ensure that the workers call the function print_consumed when they consume an item
void *consume_requests_loop(void *data)
{
  int thread_id = *((int *)data);
  int consumed_item;
  int break_outer_while = 0;

  while(1)  {
    pthread_mutex_lock(&mutex);

    // Common condition for 
    // -> Termination of worker thread
    // -> Pausing when empty buffer to read
    while(curr_buf_size == 0){ 

      // Completed the reading of all items :
      if(item_to_produce >= total_items) {

        // waking up another sleeping worker...
        pthread_cond_signal(&read_available); 
        
        break_outer_while = 1;
        break;
      }
      // If not completed :
      pthread_cond_wait(&read_available, &mutex);
    }
    if(break_outer_while) break;

    // Arrived here, if ready to consume items from buffer
    consumed_item = buffer[--curr_buf_size];
    // consumed_item = buffer[curr_buf_size--]; ----- SILLY MISTAKE
    
    print_consumed(consumed_item, thread_id);

    // Some writer threads may be waiting :
    pthread_cond_signal(&write_available);
    
    pthread_mutex_unlock(&mutex);
  }
  pthread_mutex_unlock(&mutex);

  return 0;
}

//produce items and place in buffer
//modify code below to synchronize correctly
void *generate_requests_loop(void *data)
{
  int thread_id = *((int *)data);
  int break_outer_while = 0;

  while(1)  {
    pthread_mutex_lock(&mutex);

    // stopping condition - writing is done
    if(item_to_produce >= total_items) {
      // wake up other sleeping writers
      pthread_cond_signal(&write_available); 

      break;
    }

    while(curr_buf_size >= max_buf_size){
      pthread_cond_wait(&write_available, &mutex);
      
      // possible that we need to terminate master thread
      if(item_to_produce >= total_items) {
        break_outer_while = 1;
        // wake up other sleeping writers
        pthread_cond_signal(&write_available); 

        break;
      }
    }

    if(break_outer_while) break;
    
    // we arrive here, if we 
    // are ready to write into buffer
      
    buffer[curr_buf_size++] = item_to_produce;
    print_produced(item_to_produce, thread_id);
    item_to_produce++;
    
    // Possible that worker is waiting : 
    pthread_cond_signal(&read_available);

    pthread_mutex_unlock(&mutex);
  }
  // when the loop breaks, we are still holding the lock
  pthread_mutex_unlock(&mutex);
  return 0;
}


int main(int argc, char *argv[])
{
  int *master_thread_id;
  pthread_t *master_thread;
  int *worker_thread_id;
  pthread_t *worker_thread;

  // initialising global variables
  item_to_produce = 0;
  curr_buf_size = 0;
  
  int i;
  
  if (argc < 5) {
    printf("./master-worker #total_items #max_buf_size #num_workers #masters e.g. ./exe 10000 1000 4 3\n");
    exit(1);
  }
  else {
    num_masters = atoi(argv[4]);
    num_workers = atoi(argv[3]);
    max_buf_size = atoi(argv[2]);
    total_items = atoi(argv[1]);
  }
    

  buffer = (int *)malloc (sizeof(int) * max_buf_size);

  //create master producer threads
  master_thread_id = (int *)malloc(sizeof(int) * num_masters);
  master_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_masters);
  for (i = 0; i < num_masters; i++)
    master_thread_id[i] = i;

  //create worker consumer threads
  worker_thread_id = (int *)malloc(sizeof(int) * num_workers);
  worker_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_workers);
  for (i = 0; i < num_workers; i++)
    worker_thread_id[i] = i;



  for (i = 0; i < num_masters; i++)
    pthread_create(&master_thread[i], NULL, generate_requests_loop, (void *)&master_thread_id[i]);
  
  //create worker consumer threads
  for (i = 0; i < num_workers; i++)
    pthread_create(&worker_thread[i], NULL, consume_requests_loop, (void *)&worker_thread_id[i]);

  
  //wait for all threads to complete
  for (i = 0; i < num_masters; i++)  {
    pthread_join(master_thread[i], NULL);
    printf("master %d joined\n", i);
  }
  for (i = 0; i < num_workers; i++)  {
    pthread_join(worker_thread[i], NULL);
    printf("worker %d joined\n", i);
  }
  
  /*----Deallocating Buffers---------------------*/
  free(buffer);
  free(master_thread_id);
  free(master_thread);
  free(worker_thread_id);
  free(worker_thread);
  
  return 0;
}
