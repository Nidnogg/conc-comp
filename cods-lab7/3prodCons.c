#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#define N_PROD 1
#define N_CONS 2
#define BUFFERSIZE 5

int* buffer;
int count = 0, count_fib = 1, in = 0, out = 0;

pthread_mutex_t mutex;
pthread_cond_t cond_cons, cond_prod;

// Function prototypes


void printBuffer() {
  printf("Current buffer: [ ");
  for(int i = 0; i < BUFFERSIZE; i++) 
    printf("%d ", buffer[i]);
  printf("]\n");
}

void waitFor (unsigned int secs) {
  unsigned int retTime = time(0) + secs;   // Get finishing time.
  while (time(0) < retTime);               // Loop until it arrives.
}

int fib(int n) {
  if(n == 1) return 1;
  if(n == 2) return 1;
  else return fib(n - 1) + fib(n - 2);
}

int isPrime(long unsigned int n) {
  if(n <= 1) return 0;
  if(n == 2) return 1;
  if(n % 2 == 0) return 0;
  for(int i = 3; i < sqrt(n)+1; i+=2) {
    if(n % i == 0) return 0;
  }
  return 1;
}

void insertBuffer(int elem, int tid) {
  pthread_mutex_lock(&mutex);

  // Waits if buffer is full
  while(count == BUFFERSIZE) {
    printf("Thread %d (Producer) will wait until count != BUFFERSIZE (buffer isn't full)\n", tid);
    pthread_cond_wait(&cond_prod, &mutex);
  }

  printf("Thread %d (Producer) will insert %d into buffer. Buffer before insertion:\n", tid, elem);
  printBuffer();

  buffer[in] = elem;
  in = (in + 1) % BUFFERSIZE;
  count++;

  printf("Buffer after insertion:\n");
  printBuffer(); 

  // After insertion, unlock and signal to all consumers
  pthread_mutex_unlock(&mutex);
  pthread_cond_signal(&cond_cons);
}

int * removeBuffer(int tid) {
  pthread_mutex_lock(&mutex);
  int *aux; 
  aux = malloc(sizeof(int) * BUFFERSIZE); 
  if(!aux) return -1;

  // If buffer is not full, waits in consumer queue
  while(count != 5) { 
    printf("Thread %d (Consumer) will wait until count != 0 (buffer isn't empty)\n", tid);
    pthread_cond_wait(&cond_cons, &mutex);
  }

  printf("Thread %d (Consumer) will remove %d from buffer. Buffer before removal:\n", tid, buffer[out]);
  printBuffer();

  for(int i = 0; i < BUFFERSIZE; i++) {

    aux[i] = buffer[out];
    buffer[out] = 0;
    out = (out + 1) % BUFFERSIZE;
    count--;

  }

  printf("Buffer after removal:\n");
  printBuffer(); 

  // After consumption, unlock and signal to all producers
  pthread_mutex_unlock(&mutex);
  pthread_cond_signal(&cond_prod);

  return aux;
}

void * prod(void *arg) {
  int tid = *(int *) arg;
  int elem;

  while(count_fib < 26) {
    elem = fib(count_fib);
    insertBuffer(elem, tid);
    count_fib++;
  }

  pthread_exit(NULL);
}


void * cons(void *arg) {
  int tid = *(int *) arg;
  int *removed_elems;
  removed_elems = malloc(sizeof(int) * BUFFERSIZE);
  if(!removed_elems) return -1;

  while(1) {
    removed_elems = removeBuffer(tid);
    for(int i = 0; i < BUFFERSIZE; i++) {
      if(isPrime(removed_elems[i])) {
        printf("%d (prime)\n", removed_elems[i]);
      } else {
        printf("%d\n", removed_elems[i]); 
      }

      if(removed_elems[i] == 75025) break;
    }
  }

  pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
  pthread_t* tids;

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond_cons, NULL);
  pthread_cond_init(&cond_prod, NULL);

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

  // Loops while producer/consumer threads runx
  while(1) {

    /*
    // Garbage collection
      free(tid);
      free(tids);
      pthread_mutex_destroy(&mutex);
      pthread_cond_destroy(&cond_prod);
      pthread_cond_destroy(&cond_cons);
      pthread_exit(NULL);
    }
    */

    
  }
}
