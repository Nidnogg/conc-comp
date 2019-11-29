#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int tid;
    int count;
} t_args;

// General global variables 
int reading = 0, writing = 0, waitingToWrite = 0, waitingToRead = 0, writeTurn = 0;

// Input global variables
int nWrites, nReads, NTHREADS_READ, NTHREADS_WRITE;
char* logFileName;


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
    nReads--;
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

    nWrites--;
    pthread_mutex_unlock(&mutex);
}

void * reader (void *arg) {
    int tid = * (int *) arg;

    int readId;
    int readCount;

    while(nReads > 0) {
        startRead(tid);
        
        readId = args.tid;
        readCount = args.count;
        printf("Thread Leitora de ID %d leu: id = %d e count = %d\n", tid, readId, readCount);
        
        endRead(tid);
        bobo(readId, readCount);

    }
    
    free(arg);
    pthread_exit(NULL);
}

void * writer (void *arg) {
    int tid = * (int *) arg;
    while(nWrites > 0) {
        startWrite(tid); //!! tranca outros escritores/leitores

        args.count++; // protegido em startWrite
        args.tid = tid;
        printf("Thread Escritora de ID %d modificou o struct para: id = %d e count = %d\n", tid, tid, args.count);

        endWrite(tid);
        bobo(tid, args.count);
    
    }

    free(arg);
    pthread_exit(NULL);
}
    

int main(int argc, char* argv[]) { 

    pthread_t *tids;
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&writeQueueMutex, NULL);

    pthread_cond_init(&cond_read, NULL);
    pthread_cond_init(&cond_write, NULL);
    pthread_cond_init(&cond_write_queue, NULL);

    int* tid;
    
    if(argc < 6) {
        fprintf(stderr, "Parameters: %s <# of reader threads> <# of writer threads> <# of reads> <# of writes> <logFileName.txt>\n", argv[0]);
        return 1;
    }

    nWrites = atoi(argv[1]);
    nReads = atoi(argv[2]);
    NTHREADS_READ = atoi(argv[3]);
    NTHREADS_WRITE = atoi(argv[4]);
    logFileName = argv[5];
    printf("Parameters: %s <# of reader threads> <# of writer threads> <# of reads> <# of writes> <logFileName.txt>\n", argv[0]);
    printf("Actual parameters: <# of reader threads = %d> <# of writer threads %d> <# of reads %d> <# of writes = %d> <logFileName.txt = %s>", NTHREADS_READ, NTHREADS_WRITE, nReads, nWrites, logFileName);
  
    tids = malloc(sizeof(pthread_t) * (NTHREADS_READ + NTHREADS_WRITE)); if(!tids) exit(-1);
    
    args.count = 0;

    for(int i = 0; i < NTHREADS_READ; i++) {
        tid = malloc(sizeof(int));  if(!tid) exit(-1);
        *tid = i;
        pthread_create(&tids[i], NULL, reader, (void * ) tid);
    }

    for(int i = NTHREADS_READ; i < (NTHREADS_READ + NTHREADS_WRITE);  i++) {
        tid = malloc(sizeof(int));  if(!tid) exit(-1);
        *tid = i;
        pthread_create(&tids[i], NULL, writer, (void * ) tid);
    }

    for(int i = 0; i < (NTHREADS_READ + NTHREADS_WRITE); i++) {
        if(pthread_join(tids[i], NULL)) {
            fprintf(stderr, "Failed to pthread_join(tids[%d], NULL)", i);
            return -1;
        }
    }
    free(tids);
    pthread_exit(NULL);
    
}

