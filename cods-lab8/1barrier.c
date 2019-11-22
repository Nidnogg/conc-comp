#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NTHREADS 5
#define PASSOS  5

int thread_counter = 0;
pthread_mutex_t mutex;
pthread_cond_t cond_bar;

void barrier(int nthreads) {
    pthread_mutex_lock(&mutex);
    
    thread_counter+=1;
    if(thread_counter < NTHREADS) {
        pthread_cond_wait(&cond_bar, &mutex);
    } else {
        thread_counter = 0; 
        pthread_cond_broadcast(&cond_bar);
    }
    
    pthread_mutex_unlock(&mutex);
}

void * A (void *arg) {
    int tid = * (int *) arg, i;
    int cont = 0, boba1, boba2;

    for (i=0; i < PASSOS; i++) {
        cont++;
        printf("Thread %d: cont=%d, passo=%d\n", tid, cont, i);
        //sincronizacao condicional
        barrier(NTHREADS);
        /*faz alguma coisa para gastar tempo...*/
        boba1=100; 
        boba2=-100; 
        while (boba2 < boba1) boba2++;
    }
        
    pthread_exit(NULL);}
    
int main(int agrc, char* argv[]) {
    pthread_t *tids;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_bar, NULL);

    int* tid;

    tids = malloc(sizeof(pthread_t) * NTHREADS); if(!tids) exit(-1);

    for(int i = 0; i < NTHREADS; i++) {
        tid = malloc(sizeof(int));  if(!tid) exit(-1);
        *tid = i;
        pthread_create(&tids[i], NULL, A, (void * ) tid);
    }

    free(tids);
    free(tid);
    pthread_exit(NULL);
}

