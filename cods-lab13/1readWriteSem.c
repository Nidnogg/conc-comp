#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <semaphore.h>

#define N_READER 1
#define N_WRITER 1
#define BUFFERSIZE 5

sem_t mtx_writer, mtx_reader, writer, reader; 
//semaforos
int writing=0, reading=0; //globais

// Function prototypes

void * reader_func(void *arg) {
  int tid = *(int *) arg;
  while(1) {
    sem_wait(&reader);
    sem_wait(&mtx_reader);
    reading++;

    if(reading == 1) sem_wait(&writer);
    sem_post(&mtx_reader);
    sem_post(&reader);

    printf("thread %d reading\n", tid);
    sem_wait(&mtx_reader);
    reading--;
    
    if(reading==0) sem_post(&writer);
    sem_post(&mtx_reader);
  }
  pthread_exit(NULL);
}

void * writer_func(void *arg) {
  int tid = *(int *) arg;
  while(1) {
      sem_wait(&mtx_writer);
      writing++;
      if(writing == 1) sem_wait(&reader);
      sem_post(&mtx_writer);
      sem_wait(&writer);
      //writes
      printf("thread %d writing\n", tid);

      sem_post(&writer);
      sem_wait(&mtx_writer);
      writing--; 
      if(writing == 0) sem_post(&reader);
      sem_post(&mtx_writer);
  }
  pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
  pthread_t* tids;

  int* tid;

  tids = malloc(sizeof(pthread_t) * (N_WRITER + N_READER)); if(!tids) exit(-1);

  sem_init(&mtx_writer, 0, 1);
  sem_init(&mtx_reader, 0, 1);
  sem_init(&writer, 0, 1);
  sem_init(&reader, 0, 1);


  // Creates reader threads
  for(int i = 0; i < N_READER; i++) {
    tid = malloc(sizeof(int)); if(!tid) exit(-1);
    if(pthread_create(&tids[i], NULL, reader_func, (void *) tid)) exit(-1);
  }

  // Creates writer threads
  for(int i = 0; i < N_WRITER; i++) {
    tid = malloc(sizeof(int)); if(!tid) exit(-1);
    if(pthread_create(&tids[i], NULL, writer_func, (void *) tid)) exit(-1);
  }

  // Loops while readerucer/writerumer threads runx
  while(1) {
  }
/*
  sem_destroy(&mtx_writer);
  sem_destroy(&mtx_reader);
  sem_destroy(&writer);
  sem_destroy(&reader);
 */

}
