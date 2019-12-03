#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NTHREADS 4
#define STEPS 8


sem_t me, bar; 

int threadCounter = 0;

/* The Little Book of Semaphores
void barrier(int nThreads, int tid) {
  sem_wait(&me);
  threadCounter++;
  sem_post(&me);

  if(threadCounter == NTHREADS) {
    sem_post(&bar);
  }

  sem_wait(&bar);
  sem_post(&bar);
}
*/



void barrier(int nThreads, int tid) {
  sem_wait(&me);
  threadCounter++;
  if(threadCounter < NTHREADS) {
    sem_post(&me);
    sem_wait(&bar);
  } // All threads but the last one are waiting on &bar
  threadCounter--;    // last one decrements
  if(threadCounter > 0) {   // Now that thread frees up one of the previous queue
    sem_post(&bar);         // and waits on the mutex
    sem_wait(&me);          
  } 
  // Only the last thread avoids the if block
  sem_post(&me);  // and frees up exactly one thread in the previous if block. That thread will go on to free the next one, and so forth until all have traversed the barrier.

}



void * threadFunc(void *arg) {
  int tid = * (int *) arg;

  for(int i = 0; i < STEPS; i++) {
    printf("Thread %d Hello %d!\n", tid, i+1);
    barrier(NTHREADS, tid);

  }

  pthread_exit(NULL);
}

int main(int argc, char **argv) {
  pthread_t *tids = malloc(sizeof(NTHREADS)); if(!tids) {exit(-1);}
  int *tid;

  sem_init(&me, 0, 1);
  sem_init(&bar, 0, 0);


  for(int i = 0; i < NTHREADS; i++) {
    tid = malloc(sizeof(int)); if(!tid) exit(-1);
    *tid = i;
    if(pthread_create(&tids[i], NULL, threadFunc, (void *) tid) ) {
      printf("Failed to pthread_create()!\n");
    } 
  }

  pthread_exit(NULL);
}
