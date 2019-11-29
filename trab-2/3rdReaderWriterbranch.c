#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NTHREADS_READ 2
#define NTHREADS_WRITE 2

typedef struct {
    int tid;
} t_args;

// Global variables 
int reading = 0, writing = 0, waitingToWrite = 0, waitingToRead = 0, writeTurn = 0;
int threadCount = 0;
int count;

pthread_mutex_t mutex, writeQueueMutex;
pthread_cond_t cond_read, cond_write, cond_write_queue;
t_args args;

int bobo (int id, int baba) {
    for(int i = id * 10000000; i > 0; i--) 
        baba = baba + i;
    return baba;
}

void startRead(int tid) {
    pthread_mutex_lock(&mutex);
    writeTurn = -1;
    while(writing > 0 || (waitingToWrite > 0 && writeTurn > 0)) {
        waitingToRead++;
        printf("WRITE TURN == %d Thread Leitora de ID %d irá se bloquear esperando writing > 0 (writing == %d) (waitingToRead = %d, waitingToWrite = %d)\n", writeTurn, tid, writing, waitingToRead, waitingToWrite);
        pthread_cond_wait(&cond_read, &mutex);
        waitingToRead--;
    }
    printf("Thread Leitora de ID %d segue porque writing == %d\n", tid, writing);
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

    writeTurn = 1;
    pthread_mutex_unlock(&mutex);
}

void startWrite(int tid) {
    pthread_mutex_lock(&mutex);

    // se writeTurn turno de escrita for negativo
    //, isto é, nao for a vez da escrita bloqueia tambem!
   // while(reading > 0 || writing > 0 || waitingToRead > 0 && writeTurn < 0) {
    while(reading > 0 || writing > 0 || (waitingToRead > 0 && writeTurn < 0 ))  {
        waitingToWrite++;
        printf("WRITE TURN == %d Thread Escritora de ID %d irá esperar cond_write (pois reading (%d) > 0 ou writing (%d) > 0 (waitingToRead = %d, waitingToWrite = %d) )\n", writeTurn, tid, reading, writing, waitingToRead, waitingToWrite);
        pthread_cond_wait(&cond_write, &mutex);
        waitingToWrite--;
    }

    writing++;
    pthread_mutex_unlock(&mutex);
}

void endWrite(int tid) {
    pthread_mutex_lock(&mutex);

    writing--;
    
    printf("Thread Escritora de ID %d irá sinalizar cond_write e broacastear cond_read\n", tid);

    if(waitingToWrite) pthread_cond_signal(&cond_write);
    if(waitingToRead) pthread_cond_broadcast(&cond_read);

    pthread_mutex_unlock(&mutex);
}

void * reader (void *arg) {
    int tid = * (int *) arg;

    int readId;
    int readCount;

    while(1) {
        startRead(tid);
        
        readId = args.tid;
        readCount = count;
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

        count++; // protegido em startWrite
        args.tid = tid;
        printf("Thread Escritora de ID %d modificou o struct para: id = %d e count = %d\n", tid, tid, count);

        endWrite(tid);
        bobo(tid, count);
    
    }
    pthread_exit(NULL);
}
    

int main(int agrc, char* argv[]) { 
    pthread_t *tids;

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&writeQueueMutex, NULL);

    pthread_cond_init(&cond_read, NULL);
    pthread_cond_init(&cond_write, NULL);
    pthread_cond_init(&cond_write_queue, NULL);

    int NTHREADS = NTHREADS_READ + NTHREADS_WRITE;

    int* tid;
    tids = malloc(sizeof(pthread_t) * (NTHREADS)); if(!tids) exit(-1);
    
    count = 0;

    for(int i = 0; i < NTHREADS_READ; i++) {
        tid = malloc(sizeof(int));  if(!tid) exit(-1);
        *tid = i;
        pthread_create(&tids[i], NULL, reader, (void * ) tid);
    }

    for(int i = NTHREADS_READ; i < NTHREADS;  i++) {
        tid = malloc(sizeof(int));  if(!tid) exit(-1);
        *tid = i;
        pthread_create(&tids[i], NULL, writer, (void * ) tid);
    }


    pthread_exit(NULL);
    
}

