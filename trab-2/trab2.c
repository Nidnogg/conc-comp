#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "timer.h"

// Variáveis globais gerais 
int reading = 0, writing = 0, waitingToWrite = 0, waitingToRead = 0, writeTurn = 0;
int sharedVar;  

// Variáveis globais de entrada e saída
int nReadsPerThread, nWritesPerThread, NTHREADS_READ, NTHREADS_WRITE;
char *mainFilePath;

char *commandList;

// Mutexes e condicionais
pthread_mutex_t mutex, fileMutex;
pthread_cond_t cond_read, cond_write;

// Função auxiliar pra concatenação de strings
char * concat(const char *s1, const char *s2){
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);

    char *result = malloc(len1 + len2 + 1); // +1 para terminador null
    if(!result) { fprintf(stderr, "Failed to malloc()\n"); exit(-1);}

    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1); // +1 para copiar terminador null
    return result;
}

// Auxiliar, gera filePath de cada tid leitra
char * generateFileName(int tid) {
    char fileName0[30] = "./logs/currentTestReads/";
    char fileName1[20]; 
    sprintf(fileName1, "%d.txt", tid);   

    char *filePath;
    filePath = malloc(sizeof(char) * 100);
    if(!filePath) { fprintf(stderr, "Failed to malloc filePath\n"); exit(-1); }
    
    filePath = concat(fileName0, fileName1);
    return filePath; 
}

// Seção crítica protegida por locks - Bloco de início de leitura, retorna 1 se já terminou nReads
// Aqui é assegurado que threads leitoras esperem caso as escritoras estejam ativas ou não 
// Além disso, é garantido por meio de nReads que sejam lidos exatamente quantos itens foram especificados na linha de comando
void startRead(int tid) {
    pthread_mutex_lock(&mutex);

    writeTurn = -1; // writeTurn dá conta das threads escritoras que foram desbloqueadas, mas perderam tempo da CPU antes de lerem

    // Se algum escritor estiver escrevendo, leitor espera
    // Somado a isso, para prevenir inanição, sinaliza-se também se há escritores esperando e se está no turno deles escreverem (writeTurn)
    while(writing > 0 || (waitingToWrite > 0 && writeTurn > 0)) {
        
        waitingToRead++; //Threads esperando marcam sua respectiva fila de espera

        // Atenção - cada bloco com &fileMutex indica um print. O código não é longo para prejudicar quem está lendo-o, m
        // as sim por conta de infelicidades do tratamento de strings em C, que por bem ou por mal, é um grande empecilho 
        // para muitos programadores.  
        pthread_mutex_lock(&fileMutex);
        fprintf(stdout, "Thread Leitora de ID %d irá se bloquear esperando writing > 0 (writing == %d) (waitingToRead = %d), (writeTurn = %d)\n", tid, writing, waitingToRead, writeTurn);
        char commandToAppend[40];
        sprintf(commandToAppend, "tReaderBlocked(%d, %d, %d, %d)\n", tid, writing, waitingToWrite, writeTurn);
        commandList = concat(commandList, commandToAppend);
        pthread_mutex_unlock(&fileMutex);

        pthread_cond_wait(&cond_read, &mutex);
        waitingToRead--;  // e a desmarcam ao parar de esperar
        
        pthread_mutex_lock(&fileMutex);
        fprintf(stdout, "Thread Leitora de ID %d segue porque writing == %d\n", tid, writing);
        sprintf(commandToAppend, "tReaderUnblocked(%d, %d, %d, %d)\n", tid, writing, waitingToWrite, writeTurn);
        commandList = concat(commandList, commandToAppend);
        pthread_mutex_unlock(&fileMutex);
       
    }

    pthread_mutex_lock(&fileMutex);
    fprintf(stdout, "Thread Leitora de ID %d vai ler\n", tid);
    char commandToAppend[40];
    sprintf(commandToAppend, "tReaderStartRead(%d)\n", tid);
    commandList = concat(commandList, commandToAppend);
    pthread_mutex_unlock(&fileMutex);

    reading++;  // Agora há +1 thread lendo

    pthread_mutex_unlock(&mutex);
}

// Seção crítica protegida por locks - Essa routina sinaliza o fim de leitura, liberando threads escritoras que estavam em espera, 
// e manipula writeTurn para garantir que não haja inanição
void endRead(int tid) {
    pthread_mutex_lock(&mutex);

    reading--;
    if(reading == 0) {

        pthread_mutex_lock(&fileMutex);
        fprintf(stdout, "Thread Leitora de ID %d irá sinalizar cond_write (pois reading == %d)\n", tid, reading);
        char commandToAppend[40];
        sprintf(commandToAppend, "tReaderSignalled(%d, %d)\n", tid, reading);
        commandList = concat(commandList, commandToAppend);
        pthread_mutex_unlock(&fileMutex);

        pthread_cond_signal(&cond_write);
    }

    writeTurn = 1;
    
    pthread_mutex_unlock(&mutex);
}

// Seção crítica de início de escrita - garante inanição e que threads escritoras esperem conforme a definição do problema
void startWrite(int tid) {
    pthread_mutex_lock(&mutex);

    // Não pode-se escrever enquanto outros escritores escrevem, ou outros leitores leem.
    // Inanição evitada pelas filas waitingToRead e o turno de escrita
    while(reading > 0 || writing > 0 || (waitingToRead > 0 && writeTurn < 0 ))  {
        waitingToWrite++;  //Threads esperando marcam sua respectiva fila de espera
    
        pthread_mutex_lock(&fileMutex);
        fprintf(stdout, "Thread Escritora de ID %d irá esperar cond_write (pois reading (%d) > 0 ou writing (%d) > 0 (waitingToRead = %d, writeTurn = %d) )\n", tid, reading, writing, waitingToRead, writeTurn);
        char commandToAppend[40];
        sprintf(commandToAppend, "tWriterBlocked(%d, %d, %d, %d, %d)\n", tid, reading, writing, waitingToRead, writeTurn);
        commandList = concat(commandList, commandToAppend);
        pthread_mutex_unlock(&fileMutex);

        pthread_cond_wait(&cond_write, &mutex); // e a desmarcam ao parar de esperar
        
        waitingToWrite--;
        
        pthread_mutex_lock(&fileMutex);
        fprintf(stdout, "Thread Escritora de ID %d desbloqueada pois reading = %d ou writing = %d ou (waitingToRead = %d writeTurn = %d))\n", tid, reading, writing, waitingToRead, writeTurn);
        sprintf(commandToAppend, "tWriterUnblocked(%d, %d, %d, %d, %d)\n", tid, reading, writing, waitingToRead, writeTurn);
        commandList = concat(commandList, commandToAppend);
        pthread_mutex_unlock(&fileMutex);

    }

    pthread_mutex_lock(&fileMutex);
    fprintf(stdout, "Thread Escritora de ID %d vai escrever\n", tid);
    char commandToAppend[40];
    sprintf(commandToAppend, "tWriterStartWrite(%d)\n", tid);
    commandList = concat(commandList, commandToAppend);
    pthread_mutex_unlock(&fileMutex);

    writing++;

    pthread_mutex_unlock(&mutex);
}

// Garante a liberamento das threads leitoras paradas
void endWrite(int tid) {
    pthread_mutex_lock(&mutex);
    
    writing--;

    pthread_mutex_lock(&fileMutex);
    fprintf(stdout, "Thread Escritora de ID %d irá sinalizar cond_write e broacastear cond_read\n", tid);
    char commandToAppend[40];
    sprintf(commandToAppend, "tWriterSignalledBroadcasted(%d)\n", tid);
    commandList = concat(commandList, commandToAppend);
    pthread_mutex_unlock(&fileMutex);

    if(waitingToWrite) pthread_cond_signal(&cond_write); // libera um escritor se tiver esperando
    if(waitingToRead) pthread_cond_broadcast(&cond_read); // libera todos os leitores se houver algum esperando

    pthread_mutex_unlock(&mutex);
}

// Função chamada pelas threads leitoras
void * reader (void *arg) {
    int tid = * (int *) arg;
    int readItem; 
    int nReads = nReadsPerThread;// nReads local pra indicar o término das leituras

    char *tidFilePath = generateFileName(tid);
    FILE *tidFilePointer;  // tidFilePointer é o .txt para cada thread leitora com seus valores lidos
    
    tidFilePointer = fopen(tidFilePath, "w");
    if(!tidFilePointer) {
        printf("Failed to fopen! Error: %s\n", strerror(errno)); //exit(-1);
    }


    while(nReads > 0) {
        startRead(tid);  // Seção crítica protegida por locks
        
        readItem = sharedVar;

        pthread_mutex_lock(&fileMutex);
        fprintf(stdout, "Thread %d leu %d\n", tid, readItem);
        char commandToAppend[40];
        sprintf(commandToAppend, "tRead(%d, %d)\n", tid, readItem);
        commandList = concat(commandList, commandToAppend);
        pthread_mutex_unlock(&fileMutex);
        fprintf(tidFilePointer, "%d\n", readItem);
        nReads--;   // Cada leitura é subtraída no startRead

        endRead(tid); // Seção crítica protegida por locks
        sleep(1); // Tempo de processamento - necessário de se ajustar em certos computadores para que funcione corretamente. Deve ser o mesmo para leitora e escritora.

    }
    
    // Garbage collection
    fclose(tidFilePointer);
    free(arg);

    pthread_exit(NULL);
}

// Função chamada pelas threads escritoras
void * writer (void *arg) {
    int tid = * (int *) arg;
    int nWrites = nWritesPerThread;
    
    while(nWrites > 0) {
        startWrite(tid);  // Seção crítica protegida por locks
        
        // Contexto Protegido em startWrite
        sharedVar = tid; // Escreve-se em sharedVar. Nesse trecho, todas as outras threads estarão esperando esta termina em razão dos condicionais
        pthread_mutex_lock(&fileMutex);
        fprintf(stdout, "Thread %d escreveu %d\n", tid, sharedVar);
        char commandToAppend[40];
        sprintf(commandToAppend, "tWrote(%d, %d)\n", tid, sharedVar);
        commandList = concat(commandList, commandToAppend);
        pthread_mutex_unlock(&fileMutex);
        nWrites--;

        endWrite(tid); // Seção crítica
        sleep(1); // Importante ser o mesmo tempo que em readers!
    
    }

    // Garbage collection
    free(arg);

    pthread_exit(NULL);
}
    
// Rotina principal aonde são criadas as threads
int main(int argc, char *argv[]) { 

    pthread_t *tids; // Vetor com IDs de threads
    FILE *mainFilePointer;
    double t_start, t_end, t_spent;


    // Inicialização de mutexes e condicionais
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&fileMutex, NULL);

    pthread_cond_init(&cond_read, NULL);
    pthread_cond_init(&cond_write, NULL);

    // Todos os comandos são concatenados a essa lista, que é escrita no arquivo após o término das threads
    commandList = malloc(sizeof(char *) * 10000);
    if(!commandList) exit(-1);
    
    int NTHREADS;
    int *tid; //writers tid
    
    // Caso o usuário imprima não digite o número correto de parâmetros
    if(argc < 6) {
        fprintf(stderr, "Digite: %s <# of reader threads> <# of writer threads> <# of reads> <# of writes> <mainFilePath.txt>\n", argv[0]);
        return 1;
    }

    // Leitura de parâmetros
    NTHREADS_READ = atoi(argv[1]);
    NTHREADS_WRITE = atoi(argv[2]);
    nReadsPerThread = atoi(argv[3]);
    nWritesPerThread = atoi(argv[4]);
    mainFilePath = argv[5];

    NTHREADS = NTHREADS_READ + NTHREADS_WRITE;

    // Abertura do arquivo de log   
    mainFilePointer = fopen(mainFilePath, "w");
    if(!mainFilePointer) {
        printf("Failed to fopen! Error: %s\n", strerror(errno)); //exit(-1);
    }

    fprintf(mainFilePointer, "commandLineParametersRead(%d, %d, %d, %d)\n", NTHREADS_READ, NTHREADS_WRITE, nReadsPerThread, nWritesPerThread);
    fprintf(stdout, "commandLineParametersRead(%d, %d, %d, %d)\n", NTHREADS_READ, NTHREADS_WRITE, nReadsPerThread, nWritesPerThread);

    tids = malloc(sizeof(pthread_t) * NTHREADS); if(!tids) exit(-1);
    
    sharedVar = -1; //Começa como -1.

    GET_TIME(t_start);

    // Criação de threads leitoras
    for(int i = 0; i < NTHREADS_READ; i++) {
        tid = malloc(sizeof(int));  if(!tid) exit(-1);
        *tid = i;
        pthread_create(&tids[i], NULL, reader, (void * ) tid);
    }

    // Criação de escritoras
    for(int i = NTHREADS_READ; i < NTHREADS;  i++) {
        tid = malloc(sizeof(int));  if(!tid) exit(-1);
        *tid = i;
        pthread_create(&tids[i], NULL, writer, (void * ) tid);
    }

    // Join para aguardar término
    for(int i = 0; i < NTHREADS; i++) {
        if(pthread_join(tids[i], NULL)) {
            fprintf(stderr, "Failed to pthread_join(tids[%d], NULL)", i);
            return -1;
        }
    }

    // Print sequencial de todos os logs das threads
    fprintf(mainFilePointer, "%s", commandList);

    GET_TIME(t_end);
    t_spent = t_end - t_start;
    fprintf(stdout, "Tempo de execução: %.4fs\n", t_spent);
    fprintf(mainFilePointer, "timeSpent(%.4f)\n", t_spent);
    fclose(mainFilePointer);
    
    printf("Terminando thread principal\n");
    free(tids);
    pthread_exit(NULL);
    
}

