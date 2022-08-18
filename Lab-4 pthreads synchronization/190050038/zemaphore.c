#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include "zemaphore.h"

void zem_init (zem_t *s, int value) {
  pthread_mutex_init(&s->mutex, NULL);
  pthread_cond_init(&s->cond, NULL);
  s->counter = value;

  return;
}

void zem_down(zem_t *s) {
    pthread_mutex_lock(&s->mutex);
    s->counter--;
    if(s->counter<0) pthread_cond_wait(&s->cond, &s->mutex);
    pthread_mutex_unlock(&s->mutex);

    return;
}

void zem_up(zem_t *s) {
    pthread_mutex_lock(&s->mutex);
    if(s->counter<0) pthread_cond_signal(&s->cond); //wakes up a single sleeping thread
    s->counter++;
    pthread_mutex_unlock(&s->mutex);
}
