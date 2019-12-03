#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NTHREADS 4

pthread_mutex_t mutex;
pthread_cond_t cond_bar;
int threadCounter = 0;

void barrier(int nThreads) {
  pthread_mutex_lock(&mutex);

  threadCounter++;
  while(threadCounter < NTHREADS) {
    pthread_cond_wait(&cond_bar, &mutex);
  }
  threadCounter = 0;
  pthread_cond_broadcast(&cond_bar);

  pthread_mutex_unlock(&mutex);
}
void * threadFunc(void *arg) {
  int tid = * (int *) arg;

  printf("Thread %d Hello 1!\n", tid);
  printf("Thread %d Hello 2!\n", tid);
  barrier(NTHREADS);
  printf("Thread %d Bye 1\n", tid);
  printf("Thread %d Bye 2\n", tid);

  pthread_exit(NULL);
}

int main(int argc, char **argv) {
  pthread_t *tids = malloc(sizeof(NTHREADS)); if(!tids) {exit(-1);}
  int *tid;

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond_bar, NULL);


  for(int i = 0; i < NTHREADS; i++) {
    tid = malloc(sizeof(int)); if(!tid) exit(-1);
    *tid = i;
    if(pthread_create(&tids[i], NULL, threadFunc, (void *) tid) ) {
      printf("Failed to pthread_create()!\n");
    } 
  }

  pthread_exit(NULL);
}
