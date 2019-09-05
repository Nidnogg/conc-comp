#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#define N 100000
#define NTHREADS 2

typedef struct {
    int idThread;
    int *elements;
} t_Args;
void *Increment (void *arg) {
    int i;
    t_Args *args = (t_Args *) arg;
    //printf("Running from thread %d\n", args->idThread);

    for(i = args->idThread; i < N; i += N) {
        *(args->elements + i) += 1;
        //printf("%d\n", *(args->elements + i));
    }

    /* Solução hardcoded 
    if(args->idThread == 0) {
        printf("Elements from thread 0:\n");
        for(i = 0; i < N; i+=2) {
            *(args->elements + i) += 1;
            //printf("%d\n", *(args->elements + i));
        }
    }
    if(args->idThread == 1) {
        printf("Elements from thread 1:\n");
        for(i = 1; i < N; i+=2) {
            *(args->elements + i) += 1;
           // printf("%d\n", *(args->elements + i));
        }
    }
    */

    free(arg);
    pthread_exit(NULL);
}

int main(void) {
    int t;
    int *elements;
    pthread_t tids[NTHREADS];
    t_Args *arg;
    clock_t begin = clock();

    elements = malloc(sizeof(int)*N);
    if(elements == NULL) {
        printf("Failed to malloc!\n"); exit(-1);
    }

    for(t = 0; t < N; t++) {
        *(elements + t) = t; // Qualquer número 
    }

    printf("Elements from main thread before increment:\n");
    for (t = 0; t < N; t++) printf("%d\n", *(elements + t));

    for(t = 0; t < NTHREADS; t++) {
        printf("Creating threads\n");

        // Aloca memória para argumento de cada thread
        arg = malloc(sizeof(t_Args));
        if(arg == NULL) {
            printf("Failed to malloc!\n"); exit(-1);
        }

        arg->idThread = t;
        arg->elements = elements;

        if(pthread_create(&tids[t], NULL, Increment, (void *) arg)) {
            printf("pthread_create() call error!\n"); exit(-1);
        }
    }
    
    for (t=0; t<NTHREADS; t++) {           
        if (pthread_join(tids[t], NULL)) {
            printf("pthread_join() call error!\n"); exit(-1); 
        } 
    }

    printf("Elements from main thread after increment:\n");
    for (t = 0; t < N; t++) printf("%d\n", *(arg->elements + t));
    printf("End of main thread\n");

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Execution time: %.4f\n", time_spent);
    
    pthread_exit(NULL);
    //return 0;
}