#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define NUM_THREADS 10
#define NUM_TIMES_PRINT 3

int curId;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond[NUM_THREADS]; // Not a semaphore

// pthread_cond_wait(&cond, &lock);
// pthread_cond_signal(&cond);


void* counting(int* id){
    for(int i = 0;i < NUM_TIMES_PRINT; i++){
        pthread_mutex_lock(&lock);

        while(curId!=*id)
            pthread_cond_wait(&cond[(*id)-1], &lock);
        curId = (curId)%NUM_THREADS +1;
        pthread_cond_signal(&cond[curId-1]);
        printf("%d \n", *id);

        pthread_mutex_unlock(&lock);
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    for(int i = 0; i<NUM_THREADS; i++){
        pthread_cond_init(&cond[i], NULL);
    }

    pthread_t a[NUM_THREADS];
    int *arg = malloc(sizeof(int)*NUM_THREADS);
    curId = 1;

    for(int i = 0; i<NUM_THREADS; i++){
        arg[i] = i+1;
        pthread_create(&a[i], NULL, counting, &arg[i]);
    }
        
    for(int i = 0; i<10; i++)
        pthread_join(a[i], NULL);


    return 0;
}
