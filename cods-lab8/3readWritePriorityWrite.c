#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NTHREADS_READ 2
#define NTHREADS_WRITE 2
#define NTHREADS NTHREADS_READ + NTHREADS_WRITE

typedef struct {
    int tid;
    int count;
} t_args;

//dem glob
int thread_counter = 0;
int reading = 0, writing = 0, waitingToWrite = 0;
pthread_mutex_t mutex;
pthread_cond_t cond_read, cond_write;
t_args args;

int bobo (int id, int baba) {
    for(int i = id * 100000; i > 0; i--) 
        baba = baba + i;
    return baba;
}

void startRead() {
    pthread_mutex_lock(&mutex);
    while(writing > 0 || waitingToWrite > 0) { 
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
    waitingToWrite++;
    while((reading > 0 || writing > 0)) {
        pthread_cond_wait(&cond_write, &mutex);
    }
    waitingToWrite--;
    writing++;
    pthread_mutex_unlock(&mutex);

}

void endWrite() {
    pthread_mutex_lock(&mutex);

    writing--;
    
    pthread_cond_signal(&cond_write);
    if(!waitingToWrite)
        pthread_cond_broadcast(&cond_read);

    pthread_mutex_unlock(&mutex);
}

void * reader (void *arg) {
    int tid = * (int *) arg;

    int readId;
    int readCount;

    while(1) {
        startRead();
            readId = args.tid;
            readCount = args.count;
            bobo(readId, readCount);
            printf("Thread Leitora de ID %d leu: id = %d e count = %d\n", tid, readId, readCount);
        endRead();
    }
    pthread_exit(NULL);
}

void * writer (void *arg) {
    int tid = * (int *) arg;
    while(1) {
        startWrite();
        args.count ++;
        args.tid = tid;
        printf("Thread Escritora de ID %d modificou o struct para: id = %d e count = %d\n", tid, tid, args.count);

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
    
    args.count = 0;

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

