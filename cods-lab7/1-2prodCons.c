#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#define N_PROD 1
#define N_CONS 1
#define BUFFERSIZE 5

int* buffer;
int count = 0, in = 0, out = 0;

pthread_mutex_t mutex;
pthread_cond_t cond_cons, cond_prod;

// Function prototypes
void printBuffer();

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

int removeBuffer(int tid) {
  int aux; 
  pthread_mutex_lock(&mutex);

  printf("Thread %d (Consumer) will remove %d from buffer. Buffer before removal:\n", tid, buffer[out]);
  printBuffer();
  aux = buffer[out];
  buffer[out] = 0;
  out = (out + 1) % BUFFERSIZE;
  count--;

  printf("Buffer after removal:\n");
  printBuffer(); 

  // After consumption, unlock and signal to all producers
  pthread_mutex_unlock(&mutex);
  pthread_cond_signal(&cond_prod);

  return aux;
}

void * prod(void *arg) {
  int tid = *(int *) arg;
  int elem = 5;

  // Waits if buffer is full
  while(count == BUFFERSIZE) pthread_cond_wait(&cond_prod, &mutex);
  while(1) insertBuffer(elem, tid);

  //pthread_exit(NULL);
}


void * cons(void *arg) {
  int tid = *(int *) arg;
  int removed_elem;

  // If buffer is empty, waits in consumer queue
  while(count == 0) pthread_cond_wait(&cond_cons, &mutex);

  for(;;) removed_elem = removeBuffer(tid);
  if(isPrime(removed_elem)) {
    printf("%d is prime\n", removed_elem);
  } else {
    printf("%d is not prime\n", removed_elem); 
  }

  //pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
  pthread_t* tids;

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond_cons, NULL);
  pthread_cond_init(&cond_prod, NULL);

  int* tid;

  buffer = malloc(sizeof(int) * BUFFERSIZE); if(!buffer) exit(-1);

  tids = malloc(sizeof(pthread_t) * (N_CONS + N_PROD)); if(!tids) exit(-1);

  // Initializes buffer
  for(int i = 0; i < BUFFERSIZE; i++) buffer[i] = 0;
  printBuffer();

  // Creates producer threads
  for(int i = 0; i < N_PROD; i++) {
    tid = malloc(sizeof(int)); if(!tid) exit(-1);
    if(pthread_create(&tids[i], NULL, prod, (void *) tid)) exit(-1);
  }

  // Consumer threads
  for(int i = 0; i < N_CONS; i++) {
    tid = malloc(sizeof(int)); if(!tid) exit(-1);
    if(pthread_create(&tids[i], NULL, cons, (void *) tid)) exit(-1);
  }

  // Garbage collection
  free(tid);
  free(tids);
  free(buffer);

  pthread_cond_destroy(&cond_cons);
  pthread_cond_destroy(&cond_prod);
  pthread_mutex_destroy(&mutex);
  pthread_exit(NULL);

}

void printBuffer() {
  printf("Current buffer: [ ");
  for(int i = 0; i < BUFFERSIZE; i++) 
    printf("%d ", buffer[i]);
  printf("]\n");
}

  /*
  // Joins threads
  for(i = 0; i < (N_CONS + N_PROD); i++) {
      if(pthread_join(tids[i], NULL)) exit(-1);    //failed to pthread join
  }

  // garbage collection

  free(tid);
  free(tids);
  free(buffer);

  pthread_cond_destroy(&cond_cons);
  pthread_cond_destroy(&cond_prod);
  pthread_mutex_destroy(&mutex);
  pthread_exit(NULL);
*/