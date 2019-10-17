#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NTHREADS_READ 2
#define NTHREADS_WRITE 2
#define NTHREADS NTHREADS_READ + NTHREADS_WRITE
#define NSIZE 50


int *numbers;
int thread_counter = 0;
int reading = 0, writing = 0;
pthread_mutex_t mutex;
pthread_cond_t cond_read, cond_write;


int bobo (int id, int baba) {
    for(int i = id * 100000; i > 0; i--) 
        baba = baba + i;
    return baba;
}

void startRead() {
    pthread_mutex_lock(&mutex);
    while(writing > 0) { 
        pthread_cond_wait(&cond_read, &mutex);
    }

    reading++;
    pthread_mutex_unlock(&mutex);
}

void endRead() {
    pthread_mutex_lock(&mutex);

    reading--;

    if(reading == 0) pthread_cond_signal(&cond_write);
    pthread_mutex_unlock(&mutex);
}

void startWrite() {
    pthread_mutex_lock(&mutex);
    while((reading > 0 || writing > 0)) {
        pthread_cond_wait(&cond_write, &mutex);
    }
    writing++;
    pthread_mutex_unlock(&mutex);

}

void endWrite() {
    pthread_mutex_lock(&mutex);

    writing--;
    
    pthread_cond_signal(&cond_write);
    pthread_cond_broadcast(&cond_read);

    pthread_mutex_unlock(&mutex);
}

void * reader (void *arg) {
    int tid = * (int *) arg;
    int readCount = 0;

    while(1) {
        startRead();
            readCount = 0;
            for(int i = 0; i < NSIZE; i++) {
                printf("Thread %d leu %d de numbers[%d]\n", tid, numbers[i], i);
                readCount+=numbers[i];
            }
            printf("Thread %d leu média %f\n", tid, (float)readCount/NSIZE );
            bobo(tid, readCount);
        endRead();
    }
    pthread_exit(NULL);
}

void * writer (void *arg) {
    int tid = * (int *) arg;
    while(1) {
        startWrite();
        for(int i = 0; i < NSIZE; i++) {
            if(i == 0 || i == NSIZE-1) {
                numbers[i] = tid;
                printf("Thread Escritora %d escreveu %d na posição numbers[%d]\n", tid, tid, i);
            } else {
                numbers[i] = tid * 2;
                printf("Thread Escritora %d escreveu %d na posição numbers[%d]\n", tid, 2*tid, i);

            }
        }
        endWrite();
    }
    pthread_exit(NULL);
}
    

int main(int agrc, char* argv[]) { 
    pthread_t *tids;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_read, NULL);
    pthread_cond_init(&cond_write, NULL);

    int* tid;
    tids = malloc(sizeof(pthread_t) * (NTHREADS)); if(!tids) exit(-1);
    
    numbers = malloc(sizeof(int) * NSIZE);

    for(int i = 0; i < NSIZE; i++) numbers[i] = i;
     
    for(int i = 0; i < NTHREADS_READ; i++) {
        tid = malloc(sizeof(int));  if(!tid) exit(-1);
        *tid = i;
        pthread_create(&tids[i], NULL, reader, (void * ) tid);
    }

    for(int i = 0; i < NTHREADS_WRITE; i++) {
        tid = malloc(sizeof(int));  if(!tid) exit(-1);
        *tid = i;
        pthread_create(&tids[i], NULL, writer, (void * ) tid);
    }

    free(tids);
    free(tid);
    pthread_exit(NULL);
}

