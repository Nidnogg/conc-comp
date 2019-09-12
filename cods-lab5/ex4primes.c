#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "timer.h"
#define NTHREADS 2


int nPrimes = 0;

int isPrime(long long int n);

int main(int argc, char *argv[]) {
    long long int n_elem = atoll(argv[1]);
    double start, end;

    GET_TIME(start);
    
    for(int i = 0; i < n_elem; i++) 
        if(isPrime(i)) nPrimes+=1;

    GET_TIME(end);
    
    printf("Execution time = %lf\n", end-start);
    printf("nprimes = %d\n", nPrimes);
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

