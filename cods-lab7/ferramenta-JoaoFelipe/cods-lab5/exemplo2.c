/* Disciplina: Computacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 5 */
/* Codigo: Comunicação entre threads usando variável compartilhada */
/* Exemplo com condição de corrida! */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

#define NTHREADS  2

int s = 0; //variavel compartilhada entre as threads

//funcao executada pelas threads
void *ExecutaTarefa (void *threadid) {
  int i;
  int tid = *(int*) threadid;
  printf("Thread : %d esta executando...\n", tid);
  for (i=0; i<10000; i++) {
     s++; //incrementa a variavel compartilhada 
  }
  printf("Thread : %d terminou!\n", tid);
  free(threadid);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  pthread_t tid[NTHREADS];
  int t, *id; 

  //--cria as threads
  for(t=0; t<NTHREADS; t++) {
    if ((id = malloc(sizeof(int))) == NULL) {
      printf("--ERRO: malloc()\n"); exit(-1);
    }
    *id=t;
    //printf("--Cria a thread %d\n", t);
    if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)id)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  //--espera todas as threads terminarem
  for (t=0; t<NTHREADS; t++) {
    if (pthread_join(tid[t], NULL)) {
         printf("--ERRO: pthread_join() \n"); exit(-1); 
    } 
  } 

  printf("Valor de s = %d\n", s);

  pthread_exit(NULL);
  return 1;
}
