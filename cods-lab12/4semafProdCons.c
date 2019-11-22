#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <semaphore.h>

#define N_PROD 1
#define N_CONS 1
#define BUFFERSIZE 5

int* buffer;
int in = 0, out = 0;

sem_t mutexCons, mutexProd;
sem_t slotFull, slotEmpty;

// Function prototypes


void printBuffer() {
  printf("Current buffer: [ ");
  for(int i = 0; i < BUFFERSIZE; i++) 
    printf("%d ", buffer[i]);
  printf("]\n");
}

void insertBuffer(int tid) {
  sem_wait(&slotEmpty);
  sem_wait(&mutexProd);
  buffer[in] = tid;
  in = (in + 1) % BUFFERSIZE;

  printBuffer();

  sem_post(&mutexProd);
  sem_post(&slotFull);
}

int removeBuffer(int tid) {
  int item;

  sem_wait(&slotFull);
  sem_wait(&mutexCons);


  item = buffer[out];
  buffer[out] = -1;
  out = (out + 1) % BUFFERSIZE;

  printBuffer();
  
  sem_wait(&mutexCons);
  sem_post(&slotEmpty);

  return item;
}

void * prod(void *arg) {
  while(1) {
    int tid = *(int *) arg;
    insertBuffer(tid);
  }
  pthread_exit(NULL);
}


void * cons(void *arg) {
  int tid = *(int *) arg;
  int elem;
  while(1) {
    elem = removeBuffer(tid);
  }
  pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
  pthread_t* tids;

  sem_init(&mutexCons, 0, 1);
  sem_init(&mutexProd, 0, 1);
  sem_init(&slotEmpty, 0, 0);
  sem_init(&slotFull, 0, BUFFERSIZE);

  int* tid;

  buffer = malloc(sizeof(int) * BUFFERSIZE); if(!buffer) exit(-1);

  tids = malloc(sizeof(pthread_t) * (N_CONS + N_PROD)); if(!tids) exit(-1);

  // Initializes buffer, then prints it
  for(int i = 0; i < BUFFERSIZE; i++) buffer[i] = 0;
  printBuffer();

  // Creates producer threads
  for(int i = 0; i < N_PROD; i++) {
    tid = malloc(sizeof(int)); if(!tid) exit(-1);
    if(pthread_create(&tids[i], NULL, prod, (void *) tid)) exit(-1);
  }

  // Creates consumer threads
  for(int i = 0; i < N_CONS; i++) {
    tid = malloc(sizeof(int)); if(!tid) exit(-1);
    if(pthread_create(&tids[i], NULL, cons, (void *) tid)) exit(-1);
  }

  // Loops while producer/consumer threads run
  while(1);
}
