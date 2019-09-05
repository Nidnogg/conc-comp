#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "timer.h"

#define NTHREADS 4

typedef struct { // Structure for thread arguments
  int tid;
  long long int n;
} t_Args;

double start, finish, elapsed; // Time tracking variables
double sumA = 0.0;
double sumB = 0.0;
double pi = 0.0;

void * increment(void *t_args) {
  t_Args *args = (t_Args *) t_args;
  int tid = args->tid;
  long long int n = args->n;
  long long int b_size, b_start, b_end;
  double *partial_sum;

  b_size = n/NTHREADS;
  b_start = tid * b_size;
  if(tid < NTHREADS-1) b_end = b_start + b_size;
  else b_end = n;
    
  
  partial_sum = malloc(sizeof(double)); 
  if(!partial_sum) {exit(-1); printf("Failed to malloc!");}


  if(NTHREADS == 1) {
    printf("Please run ./run_seq.sh\n");
    exit(-1);
  }

  for(long long int i = b_start; i < b_end; i++) {
    if(i % 2 == 0) {
      *partial_sum += 1.0/(2.0*i + 1.0);

    } else {
      *partial_sum -= 1.0/(2.0*i + 1.0);
    }  
  }

  free(args);
  pthread_exit((void *) partial_sum);
}

int main(int argc, char *argv[]) {

  pthread_t *tids;
  t_Args *t_args;
  long long int i;
  double *partial_sum;
  
  long long int n = atoll(argv[1]);

  GET_TIME(start);

  partial_sum = malloc(sizeof(double)); 
  if(!partial_sum) {return -1; printf("Failed to malloc!");}

  tids = malloc(sizeof(pthread_t) * NTHREADS);
  if(!tids) { return -1; printf("Failed to malloc!"); }
  

  for(i = 0; i < NTHREADS; i++) {
    t_args = malloc(sizeof(t_Args));
    if(!t_args) { return -1; printf("Failed to malloc!"); }

    t_args->tid = i;
    t_args->n = n;
    
    // Create threads
    if(pthread_create(&tids[i], NULL, increment, (void *) t_args)) {
      printf("Failed to pthread_create!\n");
      return -1;
    }
  }

  for(i = 0; i < NTHREADS; i++) {
    if(pthread_join(tids[i], (void *) &partial_sum)) {
      printf("Failed to pthread_join!\n");
      return -1;
    }
    printf("partial sum %lf\n", *partial_sum); 
    pi += *partial_sum;
    free(partial_sum);
  }

  pi = 4 * (pi);

  free(tids);

  GET_TIME(finish);
  elapsed = finish - start;
  printf("M_PI %lf pi %lf time elapsed %lf\n", M_PI, pi, elapsed);

  return 0;
}
