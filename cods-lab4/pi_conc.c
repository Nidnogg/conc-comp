#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#define NTHREADS 2

//double *sumA = 0.0;
//double *sumB = 0.0;

//implement increment function
int main(int argc, char *argv[]) {

  pthread_t *tids;
  int *tid;
  long long int i;

  double sum = 1.0;
  double aux = 0.0;
  long long int n = atoll(argv[1]);

  tids = malloc(sizeof(pthread_t) * NTHREADS);
  if(!tids) { return(-1); printf("Failed to malloc!"); }

  for(i = 0; i < NTHREADS; i++) {
    tid = malloc(sizeof(int));
    if(!tid) { return -1; printf("Failed to malloc!"); }
    *tid = i;
    
    // Create threads
    if(pthread_create(&tids[i], NULL, increment, (void *) tid)) {
      printf("Failed to pthread_create!\n");
      return -1;
    }
  }

  /*
  for(i = 0; i < n; i++) {
    if(i % 2 == 0) {
      sum -= 1.0/(aux + 3.0);
      printf("IF i %lli i mod 2 %lli %lf\n", i, i %2,  (aux + 3.0));
      aux += 2.0;
    } else {
      printf("ELSE i %lli i mod 2 %lli %lf\n", i, i %2,  (aux + 3.0));
      sum += 1.0/(aux + 3.0);
      aux += 2.0;
    }
  }

  sum = 4.0 * sum;
  printf("M_PI %lf pi %lf\n", M_PI, sum);
  */

  return 0;
}
