#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

typedef struct {
    int tid;
    int sharedVar;  
} t_args;

// General global variables 
int reading = 0, writing = 0, waitingToWrite = 0, waitingToRead = 0, writeTurn = 0;

// Input global variables
int nReads, nWrites, NTHREADS_READ, NTHREADS_WRITE;
char *logFileName;

pthread_mutex_t mutex;
pthread_cond_t cond_read, cond_write;
t_args args;

int bobo (int id, int baba) {
    for(int i = id * 10000000; i > 0; i--) 
        baba = baba + i;
    return baba;
}

char * concat(const char *s1, const char *s2){
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);

    char *result = malloc(len1 + len2 + 1); // +1 for the null-terminator
    if(!result) { fprintf(stderr, "Failed to malloc()\n"); exit(-1);}

    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1); // +1 to copy the null-terminator
    return result;
}

char * generateFileName(int tid) {
    char fileName0[30] = "./logThreadReads/tid";
    char fileName1[10]; 
    sprintf(fileName1, "%d.txt", tid);   

    char *filePath;
    filePath = malloc(sizeof(char) * 100);
    if(!filePath) { fprintf(stderr, "Failed to malloc filePath\n"); exit(-1); }
    
    filePath = concat(fileName0, fileName1);

    return filePath; 
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
    int readItem;
    FILE *filePointer;
    char *filePath = generateFileName(tid);

    printf("actual file path = %s\n", filePath);
    
    filePointer = fopen(filePath, "w");
    if(!filePointer) {
        printf("Failed to fopen! Error: %s\n", strerror(errno)); //exit(-1);
    }

    while(1) {
        startRead(tid);

        readItem = args.sharedVar;
       // fprintf(filePointer, "Thread Leitora de ID %d leu: sharedVar = %d\n", tid, readItem);
        printf("Thread Leitora de ID %d leu: sharedVar = %d\n", tid, readItem);
        
        endRead(tid);
        bobo(tid, tid);
    }
    
    free(arg);
    fclose(filePointer);

    pthread_exit(NULL);
}

void * writer (void *arg) {
    int tid = * (int *) arg;
    
    while(1) {
        startWrite(tid); //!! tranca outros escritores/leitores

        // Contexto Protegido em startWrite
        args.sharedVar = tid;
        printf("Thread Escritora de ID %d modificou o struct para: sharedVar = %d\n", tid, tid);

        endWrite(tid);
        bobo(tid, tid);
    
    }

    free(arg);
    pthread_exit(NULL);
}
    

int main(int argc, char *argv[]) { 

    pthread_t *tids;
    pthread_mutex_init(&mutex, NULL);

    pthread_cond_init(&cond_read, NULL);
    pthread_cond_init(&cond_write, NULL);

    int *tid;
    
    if(argc < 6) {
        fprintf(stderr, "Parameters: %s <# of reader threads> <# of writer threads> <# of reads> <# of writes> <logFileName.txt>\n", argv[0]);
        return 1;
    }

    NTHREADS_READ = atoi(argv[1]);
    NTHREADS_WRITE = atoi(argv[2]);
    nReads = atoi(argv[3]);
    nWrites = atoi(argv[4]);
    logFileName = argv[5];

    printf("Parameters: %s <# of reader threads> <# of writer threads> <# of reads> <# of writes> <logFileName.txt>\n", argv[0]);
    printf("Actual parameters: <# of reader threads = %d> <# of writer threads %d> <# of reads %d> <# of writes = %d> <logFileName.txt = %s>", NTHREADS_READ, NTHREADS_WRITE, nReads, nWrites, logFileName);
  
    tids = malloc(sizeof(pthread_t) * (NTHREADS_READ + NTHREADS_WRITE)); if(!tids) exit(-1);
    
    args.sharedVar = -1; //Começa como -1.
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

