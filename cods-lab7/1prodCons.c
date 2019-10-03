#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#define NTHREADS 2
#define BUFFERSIZE 5

int* buffer;
int count = 0, count_fib = 1, in = 0, out = 0;

pthread_mutex_t t_mutex;
pthread_cond_t t_cond_cons, t_cond_prod;


int fib(int n);
int isPrime(int n);

void * cons (void* arg) {
    int elem;
    pthread_mutex_lock(&t_mutex);
    if(count == 0) {
        pthread_cond_wait(&t_cond_prod, &t_mutex);
    }
    count--;
    elem = buffer[out];
    isPrime(elem);

    out = (out + 1) % 5;

    pthread_cond_signal(&t_cond_cons);
    pthread_mutex_unlock(&t_mutex);

    pthread_exit(NULL);
}

void * prod (void* arg) {
    pthread_mutex_lock(&t_mutex);
    if(count_fib <= 25) {
        if(count == BUFFERSIZE) {                  // works for two threads
            pthread_cond_wait(&t_cond_cons, &t_mutex); // waits until buffer isn't full
        }
        count++;
        for(int i = 1; i <= 25; i++) {
            buffer[in] = fib(count_fib);
            count_fib +=1;
            in = (in + 1) % BUFFERSIZE;
        }

    }

    pthread_mutex_unlock(&t_mutex);
    pthread_cond_signal(&t_cond_prod);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    pthread_t* tids;
    
    pthread_mutex_init(&t_mutex, NULL);
    pthread_cond_init(&t_cond_cons, NULL);
    pthread_cond_init(&t_cond_prod, NULL);

    int i = 0;
    int* tid;
    
    buffer = malloc(sizeof(int) * BUFFERSIZE); if(!buffer) exit(-1);
    
    tids = malloc(sizeof(pthread_t) * NTHREADS); if(!tids) exit(-1);

    tid = malloc(sizeof(int)); if(!tid) exit(-1);
    if(pthread_create(&tids[0], NULL, prod, (void *) tid)) exit(-1);

    tid = malloc(sizeof(int)); if(!tid) exit(-1);
    if(pthread_create(&tids[1], NULL, cons, (void *) tid)) exit(-1);
    
    for(i = 0; i < NTHREADS; i++) {
        if(pthread_join(tids[i], NULL)) exit(-1);    //failed to pthread join
    }

    printf("Printing buffer\n");
    for(int i = 0; i < BUFFERSIZE; i++) 
        printf("%d\n", buffer[i]);
    

    // garbage collection

    free(tid);
    free(tids);
    free(buffer);

    pthread_cond_destroy(&t_cond_cons);
    pthread_cond_destroy(&t_cond_prod);
    pthread_mutex_destroy(&t_mutex);
    pthread_exit(NULL);
}

int fib(int n) {
    if(n == 1) return 1;
    if(n == 2) return 1;
    else return fib(n - 1) + fib(n - 2);
}

int isPrime(int n) {
    int i;
    if(n <= 1) {
        printf("%d \n", n);

        return 0;
    } 
    if(n == 2) {
        printf("%d (prime)\n", n);

        return 1;
    }
    if(n % 2 == 0) {
        printf("%d \n", n);
        return 0;
    } 
    for(i = 3; i < sqrt(n)+1; i += 2) {
        printf("%d \n", n);
        if(n % i == 0) return 0;
    }
    printf("%d (prime)\n", n);

    return 1;
}