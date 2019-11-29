#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

// General global variables 
int reading = 0, writing = 0, waitingToWrite = 0, waitingToRead = 0, writeTurn = 0;
int sharedVar;  

// Input global variables
int nReads, nWrites, NTHREADS_READ, NTHREADS_WRITE;
char *mainFilePath;

pthread_mutex_t mutex;
pthread_cond_t cond_read, cond_write;

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
    char fileName0[30] = "./logs/tid";
    char fileName1[10]; 
    sprintf(fileName1, "%d.txt", tid);   

    char *filePath;
    filePath = malloc(sizeof(char) * 100);
    if(!filePath) { fprintf(stderr, "Failed to malloc filePath\n"); exit(-1); }
    
    filePath = concat(fileName0, fileName1);

    return filePath; 
}

int startRead(int tid, FILE *mainFilePointer) {
    pthread_mutex_lock(&mutex);

    while(nReads <= 0) {
        pthread_mutex_unlock(&mutex);
        return 1;
    }

    writeTurn = -1;

    while(writing > 0 || (waitingToWrite > 0 && writeTurn > 0)) {
        while(nReads == 0) {
            pthread_mutex_unlock(&mutex);
            return 1;
        }
        waitingToRead++;
        fprintf(mainFilePointer, "Thread Leitora de ID %d irá se bloquear esperando writing > 0 (writing == %d) (waitingToRead = %d, waitingToWrite = %d)\n", tid, writing, waitingToRead, waitingToWrite);
        pthread_cond_wait(&cond_read, &mutex);
        waitingToRead--;
    }

    fprintf(mainFilePointer, "Thread Leitora de ID %d segue porque writing == %d\n nReads/Writes = %d/%d", tid, writing, nReads, nWrites);
    reading++;
    nReads--;

    pthread_mutex_unlock(&mutex);
    return 0;
}

void endRead(int tid,  FILE *mainFilePointer) {
    pthread_mutex_lock(&mutex);

    reading--;
    if(reading == 0) {
        fprintf(mainFilePointer, "Thread Leitora de ID %d irá sinalizar cond_write (pois reading == %d) e nReads/Writes = %d/%d\n", tid, reading, nReads, nWrites);
        pthread_cond_signal(&cond_write);
    }

    writeTurn = 1;
    
    pthread_mutex_unlock(&mutex);
}

int startWrite(int tid,  FILE *mainFilePointer) {
    pthread_mutex_lock(&mutex);

    while(nWrites == 0) {
        pthread_mutex_unlock(&mutex);
        return 1;
    }
  
    // se writeTurn turno de escrita for negativo
    //, isto é, nao for a vez da escrita bloqueia tambem!
    while(reading > 0 || writing > 0 || (waitingToRead > 0 && writeTurn < 0 ))  {
        while(nWrites == 0) {
            pthread_mutex_unlock(&mutex);
            return 1;
        }
  
        waitingToWrite++;
        fprintf(mainFilePointer, "Thread Escritora de ID %d irá esperar cond_write (pois reading (%d) > 0 ou writing (%d) > 0 (waitingToRead = %d, waitingToWrite = %d) )\n", tid, reading, writing, waitingToRead, waitingToWrite);
        pthread_cond_wait(&cond_write, &mutex);
        waitingToWrite--;
    }

    writing++;
    nWrites--;

    pthread_mutex_unlock(&mutex);
    return 0;
}

void endWrite(int tid,  FILE *mainFilePointer) {
    pthread_mutex_lock(&mutex);
    
    writing--;
    fprintf(mainFilePointer, "Thread Escritora de ID %d irá sinalizar cond_write e broacastear cond_read e nReads/Writes = %d/%d\n", tid, nReads, nWrites);

    if(waitingToWrite) pthread_cond_signal(&cond_write);
    if(waitingToRead) pthread_cond_broadcast(&cond_read);

    pthread_mutex_unlock(&mutex);
}

void * reader (void *arg) {
    int tid = * (int *) arg;
    int readItem;

    char *tidFilePath = generateFileName(tid);
    FILE *mainFilePointer;
    FILE *tidFilePointer;
    
    mainFilePointer = fopen(mainFilePath, "a");
    if(!mainFilePointer) {
        printf("Failed to fopen! Error: %s\n", strerror(errno)); //exit(-1);
    }

    tidFilePointer = fopen(tidFilePath, "w");
    if(!tidFilePointer) {
        printf("Failed to fopen! Error: %s\n", strerror(errno)); //exit(-1);
    }


    while(1) {
        if(startRead(tid, mainFilePointer)) {
            break;
        }

        readItem = sharedVar;
        fprintf(tidFilePointer, "Thread Leitora de ID %d leu: sharedVar = %d\n", tid, readItem);
        fprintf(mainFilePointer, "Thread Leitora de ID %d leu: sharedVar = %d\n", tid, readItem);

        endRead(tid, mainFilePointer);
        sleep(1);

    }
    
    free(arg);
    fclose(mainFilePointer);
    fclose(tidFilePointer);


    pthread_exit(NULL);
}

void * writer (void *arg) {
    int tid = * (int *) arg;
    FILE *mainFilePointer;
    
    mainFilePointer = fopen(mainFilePath, "a");
    if(!mainFilePointer) {
        printf("Failed to fopen! Error: %s\n", strerror(errno)); //exit(-1);
    }
    
    while(1) {
        if(startWrite(tid, mainFilePointer)) {
            break;
        }

        // Contexto Protegido em startWrite
        sharedVar = tid;
        fprintf(mainFilePointer, "Thread Escritora de ID %d escreveu sharedVar = %d\n", tid, sharedVar);

        endWrite(tid, mainFilePointer);
        sleep(1);
    
    }

    free(arg);
    fclose(mainFilePointer);
    pthread_exit(NULL);
}
    

int main(int argc, char *argv[]) { 

    pthread_t *tids;
    pthread_mutex_init(&mutex, NULL);

    pthread_cond_init(&cond_read, NULL);
    pthread_cond_init(&cond_write, NULL);

    int NTHREADS;
    int *tid; //writers tid
    
    if(argc < 6) {
        fprintf(stderr, "Parameters: %s <# of reader threads> <# of writer threads> <# of reads> <# of writes> <mainFilePath.txt>\n", argv[0]);
        return 1;
    }

    NTHREADS_READ = atoi(argv[1]);
    NTHREADS_WRITE = atoi(argv[2]);
    nReads = atoi(argv[3]);
    nWrites = atoi(argv[4]);
    mainFilePath = argv[5];

    NTHREADS = NTHREADS_READ + NTHREADS_WRITE;

    printf("Parameters: %s <# of reader threads> <# of writer threads> <# of reads> <# of writes> <mainFilePath.txt>\n", argv[0]);
    printf("Actual parameters: <# of reader threads = %d> <# of writer threads %d> <# of reads %d> <# of writes = %d> <mainFilePath.txt = %s>", NTHREADS_READ, NTHREADS_WRITE, nReads, nWrites, mainFilePath);
  
    tids = malloc(sizeof(pthread_t) * NTHREADS); if(!tids) exit(-1);
    
    sharedVar = -1; //Começa como -1.
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

    for(int i = 0; i < NTHREADS; i++) {
        if(pthread_join(tids[i], NULL)) {
            fprintf(stderr, "Failed to pthread_join(tids[%d], NULL)", i);
            return -1;
        }
    }

    free(tids);
    pthread_exit(NULL);
    
}

