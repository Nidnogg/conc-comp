#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NTHREADS 4

// globals
pthread_mutex_t t_mutex;
pthread_cond_t t_cond;
int thread_counter = 0;

void * hola(void* tid) {
    printf("thread hola started\n");
    pthread_mutex_lock(&t_mutex);

    printf("Hola!\n");
    thread_counter+=1; 
    if(thread_counter == 2) {
        pthread_cond_broadcast(&t_cond);
    }

    printf("thread hola ended\n");
    pthread_mutex_unlock(&t_mutex);
    pthread_exit(NULL);
}

void * tudoBem(void* tid) {
    printf("thread tudoBem started\n");
    pthread_mutex_lock(&t_mutex);
    printf("Tudo bem?\n");

    thread_counter+=1; 
    if(thread_counter == 2) {
        pthread_cond_broadcast(&t_cond);
    }

    printf("thread tudoBem ended\n");
    pthread_mutex_unlock(&t_mutex);
    pthread_exit(NULL);
}

void * ate(void* tid) {
    printf("thread ate started\n");
    pthread_mutex_lock(&t_mutex);
    if(thread_counter < 2) { //aqui tanto faz if, while é necessario somente em waits em que o estado pode mudar pra desfavoravel
        pthread_cond_wait(&t_cond, &t_mutex);
    }
    printf("Até mais\n");

    printf("thread ate ended\n");
    pthread_mutex_unlock(&t_mutex);
    pthread_exit(NULL);
}

void * tchau(void* tid) {
    printf("thread tchau started\n");
    pthread_mutex_lock(&t_mutex);
    if(thread_counter < 2) {  //aqui tanto faz if, while é necessario somente em waits!!
        pthread_cond_wait(&t_cond, &t_mutex);
    }
    printf("Tchau\n");

    printf("thread tchau ended\n");
    pthread_mutex_unlock(&t_mutex);
    pthread_exit(NULL);
}
int main(int argc, char* argv[]) {
    //vetor de threads
    pthread_t* tids = malloc(sizeof(pthread_t) * NTHREADS); 
    if(!tids) { printf("Failed to malloc()\n"); exit(-1); }
    int i;
    int* tid;

    // create threads
    for(i = 0; i < NTHREADS; i++) {
        tid = malloc(sizeof(int)); 
        if(!tid) { printf("Failed to malloc()\n"); exit(-1); }
        *tid = i;
        switch(i) {
            case 0:
                pthread_create(&tids[i], NULL, tudoBem, (void *) tid);
            break;

            case 1:
                pthread_create(&tids[i], NULL, hola, (void *) tid);
            break;

            case 2:
                pthread_create(&tids[i], NULL, ate, (void *) tid);
            break;
        
            case 3:
                pthread_create(&tids[i], NULL, tchau, (void *) tid);
            break;

        }
    }


    for(i = 0; i < NTHREADS; i++) {
        pthread_join(tids[i], NULL);
    }
    

    printf ("Main thread ended\n");
    /* Desaloca variaveis e termina */
    pthread_mutex_destroy(&t_mutex);
    pthread_cond_destroy(&t_cond);
    pthread_exit (NULL); 
}