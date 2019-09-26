#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "timer.h"

#define NTHREADS 3

int nPrimes = 0;
long long int n_elem;
pthread_mutex_t mutex;

void * checkPrime(void *tid);
int isPrime(long long int n);

int main(int argc, char *argv[]) {
    pthread_t tid[NTHREADS];
    int t, id[NTHREADS];

    n_elem = atoll(argv[1]);
    double start, end;

    GET_TIME(start);

    pthread_mutex_init(&mutex, NULL);

    for(t = 0; t < NTHREADS; t++) {
        id[t] = t;
        if (pthread_create(&tid[t], NULL, checkPrime, (void *) &id[t])) {
            printf("--ERROR: pthread_create()\n"); exit(-1);
        }
    }

    for (t=0; t<NTHREADS; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join() \n"); exit(-1); 
        } 
    } 

    pthread_mutex_destroy(&mutex);


    GET_TIME(end);
    
    printf("Execution time = %lf\n", end-start);
    printf("nprimes = %d\n", nPrimes);
    pthread_exit(NULL);
}

void * checkPrime(void *thread_id) {
    int tid = *(int *) thread_id;
    //printf("Running thread : %d...\n", tid);
    
    for(int i = tid; i < n_elem; i+=NTHREADS) {
        if(isPrime(i)) {
            pthread_mutex_lock(&mutex);
            nPrimes+=1;
            pthread_mutex_unlock(&mutex);
        }
    }
    pthread_exit(NULL);

}

int isPrime(long long int n) {
    if(n<=1) return 0;
    if(n==2) return 1;
    if(n%2==0) return 0;
    for(int i = 3; i < sqrt(n)+1; i+=2)
        if(n%i==0) return 0;
    return 1;   
}



