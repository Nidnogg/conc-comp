/* Disciplina: Computacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 12 */
/* Codigo: Comunicação entre threads usando variável compartilhada e sincronização com semáforos */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <semaphore.h>

#define NTHREADS 4

// Variaveis globais
int x = 0;      //variavel compartilhada entre as threads
sem_t condHello, condOla;     //semaforos para sincronizar a ordem de execucao das threads

// Função threads
void * beep (void *threadId) {
    int tid = * (int*) threadId;
    switch(tid) {
        case 1:
            printf("hello\n");
            sem_post(&condHello);
            sem_post(&condHello);
            pthread_exit(NULL);
            break;
        case 2:
            printf("olá\n");
            sem_post(&condOla);
            sem_post(&condOla);
            pthread_exit(NULL);
            break;
        case 3:
            sem_wait(&condHello);
            sem_wait(&condOla);
            printf("bye\n");

            pthread_exit(NULL);
            break;
        case 4:
            sem_wait(&condHello);
            sem_wait(&condOla);
            printf("tchau\n");

            pthread_exit(NULL);
            break;
        default:
            pthread_exit(NULL);
            break;
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  pthread_t tid[NTHREADS];
  int *id[4], t;

  for (t=0; t<NTHREADS; t++) {
    if ((id[t] = malloc(sizeof(int))) == NULL) {
       pthread_exit(NULL); return 1;
    }
    *id[t] = t+1;
  }

  //inicia os semaforos
  sem_init(&condHello, 0, 0);

  //cria threads 
  if (pthread_create(&tid[0], NULL, beep, (void *)id[0])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
  if (pthread_create(&tid[1], NULL, beep, (void *)id[1])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
  if (pthread_create(&tid[2], NULL, beep, (void *)id[2])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
  if (pthread_create(&tid[3], NULL, beep, (void *)id[3])) { printf("--ERRO: pthread_create()\n"); exit(-1); }

  //--espera todas as threads terminarem
  for (t=0; t<NTHREADS; t++) {
    if (pthread_join(tid[t], NULL)) {
         printf("--ERRO: pthread_join() \n"); exit(-1); 
    } 
    free(id[t]);
  } 
  pthread_exit(NULL);
}
