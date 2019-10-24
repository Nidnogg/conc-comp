/* Disciplina: Computacao Concorrente */
/* Grupo: Henrique Vermelho de Toledo, João Pedro Lopes Murtinho*/
/* Prof.: Silvana Rossetto */
/* Trabalho de Implementação */
/* Codigo: Solução Concorrente do Método de Integração Numérica Retangular usando Quadratura Adaptativa */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <math.h>
#include "timer.h"

#define NTHREADS 2

pthread_mutex_t t_mutex;

float function1(float x);
float function2(float x);
float function3(float x);
float function4(float x);
float function5(float x);
float function6(float x);
float function7(float x);
//cria a estrutura de dados para armazenar os argumentos da thread
typedef struct {
   int idThread;
   float a, b;
} t_Args;

//funcao executada pelas threads
void *compute_area (void *arg) {
  t_Args *args = (t_Args *) arg;

  args->a = 10;
  args->b = 20;
  printf("Thread %d has a = %f, b = %f\n", args->idThread, args->a, args->b);
  free(arg);

  pthread_exit(NULL);
}

//funcao principal do programa
int main(int argc, char* argv[]) {
  pthread_t tids[NTHREADS];

  t_Args *arg; //receberá os argumentos para a thread
  float a, b; //Intervalo de integração
  float err;
  float result;
  int input;
  double t_start, t_end, t_spent;
 
  GET_TIME(t_start);

  // Input
  if(argc < 4) {
    fprintf(stderr, "Parameters: %s <start of interval(a)> <end of interval(b)> <max error(err)>  \n", argv[0]);
    return 1;
  }

  a = atof(argv[1]);
  b = atof(argv[2]);
  err = atof(argv[3]);

  
  printf("Select test function(s):\n" 
         "1) 1 + x\n"
         "2) sqrt(1 − (x**2)), −1 < x < 1\n"
         "3) sqrt(1 + x**4)\n"
         "4) sin(x**2)\n"
         "5) cos(e^(−x))\n"
         "6) cos(e^(−x))∗x\n"
         "7) cos(e^(−x))∗(0.005 ∗ (x**3) + 1)\n");

  scanf("%d", &input);

  // Quits if user input is incorrect
  if(input < 1 || input > 7) { 
    printf("Invalid input\n"); 
    exit(-1);
  }

  if(input == 2){
      if((a <= -1 || b <= -1) || (a >= 1 || b >= 1)){
          printf("Invalid interval\n");
          exit(-1);
      }
  }
  // Creates threads
  for(int i = 0; i < NTHREADS; i++) {
    arg = malloc(sizeof(t_Args));
    if (arg == NULL) {
      printf("--ERRO: malloc()\n"); exit(-1);
    }
    arg->idThread = i; 
    
    if (pthread_create(&tids[i], NULL, compute_area, (void *) arg)) {
      printf("Failed to pthread_create()\n"); exit(-1);
    }
  }

  for(int i = 0; i < NTHREADS; i++) {
    if(pthread_join(tids[i], NULL)) {
      printf("Failed to pthread_create()\n!"); exit(-1);
    }
  }

/*
  while(fabs(current_area_large - (current_area_small1 + current_area_small2)) > err ) {

  }
*/

  
  //printf("Estimated area: %f\n", result);
  GET_TIME(t_end);

  t_spent = t_end - t_start;
  printf("Execution time: %f seconds\n", t_spent);
    

  printf("Main thread finished\n");
  pthread_exit(NULL);
}

float function1(float x){
    return (1 + x);
}
float function2(float x){
    return sqrt(1 - pow(x, 2));
}
float function3(float x){
    return sqrt(1 + pow(x, 4));
}
float function4(float x){
    return sin(pow(x, 2));
}
float function5(float x){
    return cos(pow(M_E, x * -1));
}
float function6(float x){
    return cos(pow(M_E, x * -1)) * x;
}
float function7(float x){
    return cos(pow(M_E, x * -1)) * (0.005 * pow(x, 3) + 1);
}