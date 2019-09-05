/* Disciplina: Computacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 1 */
/* Codigo: "Hello World" usando threads em C com passagem de um argumento */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

#define NTHREADS  10

//funcao executada pelas threads
void *PrintHello (void *arg) {
  int idThread = *(int *) arg;

  printf("Hello World da thread: %d\n", idThread);
  free(arg);

  pthread_exit(NULL);
}

//funcao principal do programa
int main() {
  pthread_t tid_sistema[NTHREADS];
  int t;
  int *arg; //receberá o argumento para a thread
  //int tid[NTHREADS];

  for(t=0; t<NTHREADS; t++) {
    //printf("--Aloca e preenche argumento com o identificador da thread na aplicacao %d\n", t);
    arg = malloc(sizeof(int));
    if (arg == NULL) {
      printf("--ERRO: malloc()\n"); exit(-1);
    }
    *arg = t; 
    printf("--Argumento: %d -> alocado para a thread %d no endereço %p:\n", *arg, t, arg);
    
    printf("--Cria a thread %d\n", t);
    //tid[t] = t;
    //if (pthread_create(&tid_sistema[t], NULL, PrintHello, (void*) &tid[t])) {
    if (pthread_create(&tid_sistema[t], NULL, PrintHello, (void*) arg)) {
    //if (pthread_create(&tid_sistema[t], NULL, PrintHello, (void*) &t)) { //!!pode gerar erros!!
      printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  printf("--Thread principal terminou\n");
  pthread_exit(NULL);
}
