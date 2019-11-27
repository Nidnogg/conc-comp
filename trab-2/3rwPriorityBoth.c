#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NTHREADS_READ 2
#define NTHREADS_WRITE 2
#define NTHREADS NTHREADS_READ + NTHREADS_WRITE

typedef struct {
    int tid;
    int count;

} t_args;

typedef struct node {
    int val;
    struct node *next;
} node_t;

// Global variables 
int reading = 0, writing = 0, waitingToWrite = 0, waitingToRead = 0;
int threadCount = 0;

pthread_mutex_t mutex, writeQueueMutex;
pthread_cond_t cond_read, cond_write, cond_write_queue;
t_args args;
node_t *head = NULL;


void enqueue(node_t **head, int val) {
    node_t *new_node = malloc(sizeof(node_t));
    if (!new_node) return;

    new_node->val = val;
    new_node->next = *head;

    *head = new_node;
}

int dequeue(node_t **head) {
    node_t *current, *prev = NULL;
    int retval = -1;

    if (*head == NULL) return -1;

    current = *head;
    while (current->next != NULL) {
        prev = current;
        current = current->next;
    }

    retval = current->val;
    free(current);

    if (prev)
        prev->next = NULL;
    else
        *head = NULL;

    return retval;
}

void print_list(node_t *head) {
    node_t *current = head;

    while (current != NULL) {
        printf("%d\n", current->val);
        current = current->next;
    }
}

int bobo (int id, int baba) {
    for(int i = id * 10000000; i > 0; i--) 
        baba = baba + i;
    return baba;
}



void startRead(int tid) {
    pthread_mutex_lock(&mutex);

    if(writing > 0 || waitingToWrite > 0) {
        waitingToRead++;
        printf("Thread Leitora de ID %d irá se bloquear esperando writing > 0 (writing == %d) (waitingToRead = %d, waitingToWrite = %d)\n", tid, writing, waitingToRead, waitingToWrite);
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

    pthread_mutex_unlock(&mutex);
}

void startWrite(int tid) {
    pthread_mutex_lock(&mutex);
    if(tid == 0) pthread_cond_wait(&cond_write_queue, &mutex);
    /*
    //if(queue is empty) {
        printf("queueing thread %d because queue is empty. SIGNALING WRITE QUEUE.\n", tid);
        enqueue(&head, tid);
        pthread_cond_signal(&cond_write_queue);
    } else if(dequeue(&head) == tid) {
            printf("thread %d wil wait because it was the last dequeued thread.\n", tid);
            pthread_cond_wait(&cond_write_queue, &mutex);
            enqueue(&head, tid);
    } else {
        printf("thread %d will signal write queue and be queued.\n", tid);

        pthread_cond_signal(&cond_write_queue);
        enqueue(&head, tid);
    }    
     */



    if(reading > 0 || writing > 0 || waitingToRead > 0) {
        waitingToWrite++;
        printf("Thread Escritora de ID %d irá esperar cond_write (pois reading (%d) > 0 ou writing (%d) > 0 (waitingToRead = %d, waitingToWrite = %d) )\n", tid, reading, writing, waitingToRead, waitingToWrite);
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

        if(tid == 1) {
            startWrite(tid); //!! tranca outros escritores/leitores

            args.count++; // protegido em startWrite
            args.tid = tid;
            printf("Thread Escritora de ID %d modificou o struct para: id = %d e count = %d\n", tid, tid, args.count);

            endWrite(tid);
            bobo(tid, args.count);
        }
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

