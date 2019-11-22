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

// Global variables 
int reading = 0, writing = 0, waitingToWrite = 0, waitingToRead = 0;
pthread_mutex_t mutex;
pthread_cond_t cond_read, cond_write;
t_args args;

int bobo (int id, int baba) {
    for(int i = id * 100000; i > 0; i--) 
        baba = baba + i;
    return baba;
}

void startRead(int tid) {
    pthread_mutex_lock(&mutex);

    while(writing > 0 || waitingToWrite > 0) { 
        printf("Thread Leitora de ID %d irá se bloquear esperando writing > 0 (writing == %d) e waitingToWrite\n", tid, writing);
        waitingToRead++;
        pthread_cond_wait(&cond_read, &mutex);
    }
    printf("Thread Leitora de ID %d se desbloqueou porque writing == %d\n", tid, writing);
    waitingToRead--;
    reading++;

    pthread_mutex_unlock(&mutex);
}

void endRead(int tid) {
    pthread_mutex_lock(&mutex);

    reading--;
    if(reading == 0) {
        printf("Thread Leitora de ID %d irá sinalizar cond_write (pois reading == %d)\n", tid, reading);
        pthread_cond_signal(&cond_write);
    }

    pthread_mutex_unlock(&mutex);
}

void startWrite(int tid) {
    pthread_mutex_lock(&mutex);

    while(reading > 0 || writing > 0 || waitingToRead > 0) {
        printf("Thread Escritora de ID %d irá esperar cond_write (pois reading (%d) > 0 ou writing (%d) > 0 ou waitingToRead!)\n", tid, reading, writing);
        waitingToWrite++;
        pthread_cond_wait(&cond_write, &mutex);
    }
    waitingToWrite--;
    writing++;

    pthread_mutex_unlock(&mutex);

}

void endWrite(int tid) {
    pthread_mutex_lock(&mutex);

    writing--;
    
    printf("Thread Escritora de ID %d irá sinalizar cond_write e broacastear cond_read\n", tid);

    pthread_cond_signal(&cond_write);
    if(!waitingToWrite || waitingToRead) pthread_cond_broadcast(&cond_read);

    pthread_mutex_unlock(&mutex);
}

void * reader (void *arg) {
    int tid = * (int *) arg;

    int readId;
    int readCount;

    while(1) {
        startRead(tid);
        
        readId = args.tid;
        readCount = args.count;
        printf("Thread Leitora de ID %d leu: id = %d e count = %d\n", tid, readId, readCount);
        
        endRead(tid);
        bobo(readId, readCount);
    }
    pthread_exit(NULL);
}

void * writer (void *arg) {
    int tid = * (int *) arg;
    while(1) {
        startWrite(tid); //!! tranca outros escritores/leitores

        args.count++; // protegido em startWrite
        args.tid = tid;
        printf("Thread Escritora de ID %d modificou o struct para: id = %d e count = %d\n", tid, tid, args.count);

        endWrite(tid);
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
    free(tid); // NAS THREADS
    pthread_exit(NULL);
    
}

