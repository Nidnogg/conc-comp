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

float function1(float x);
float function2(float x);
float function3(float x);
float function4(float x);
float function5(float x);
float function6(float x);
float function7(float x);

void barrier(int nthreads) {
    pthread_mutex_lock(&t_mutex);
    
    threads_waiting+=1;
    if(threads_waiting < NTHREADS) {
        pthread_cond_wait(&t_cond_bar, &mutex);
    } else {
        threads_waiting = 0;
        update_rects();
        pthread_cond_broadcast(&t_cond_bar);
    }
    
    pthread_mutex_unlock(&t_mutex);
}

void update_rects() {
  for(int i = 0; i < NTHREADS; i++) {

    //rects[i]->a = 
    //rects[i]->b = 
    //rects[i]->area
  }
}
//cria a estrutura de dados para armazenar os argumentos da thread
typedef struct {
   int idThread;
   int input_function_type;
} Args_t;

typedef struct {
  float area, a, b;
} Rect_t;

// globals
pthread_mutex_t t_mutex;
pthread_cond_t t_cond_bar;

Rect_t rects[NTHREADS];
int threads_waiting = 0;

//funcao executada pelas threads
void * compute_area (void *arg) {
  Args_t *args = (Args_t *) arg;
  

  while(1) {
    //flag começou
    barrier(NTHREADS);
    printf("thread %d começou\n", args->idThread);

  }
  printf("Thread %d\n", args->idThread);
 
  free(arg);
  pthread_exit(NULL);
}

//main

int main(int argc, char* argv[]) {
  pthread_t tids[NTHREADS];

  Args_t *arg; //receberá os argumentos para a thread
  float a, b; //Intervalo de integração
  float err;
  float result;
  float current_area_large, current_area_small_sum;
  int input;
  double t_start, t_end, t_spent;
 
  pthread_mutex_init(&t_mutex, NULL);
  pthread_cond_init(&t_cond_bar, NULL);

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

  // Quits if user input is incorRect_t
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
    arg = malloc(sizeof(Args_t));
    if (arg == NULL) {
      printf("--ERRO: malloc()\n"); exit(-1);
    }
    arg->idThread = i; 
    arg->input_function_type = input;
    if (pthread_create(&tids[i], NULL, compute_area, (void *) arg)) {
      printf("Failed to pthread_create()\n"); exit(-1);
    }
  }

  current_area_large = INFINITY; 
  current_area_small_sum = 0;
 /*
  while(fabs(current_area_large - current_area_small_sum) > err ) {
    pthread_mutex_lock(&t_mutex);
    pthread_mutex_unlock(&t_mutex);
  }
 */
 

  for(int i = 0; i < NTHREADS; i++) {
    if(pthread_join(tids[i], NULL)) {
      printf("Failed to pthread_create()\n!"); exit(-1);
    }
  }

  //printf("Estimated area: %f\n", result);
  GET_TIME(t_end);

  t_spent = t_end - t_start;
  printf("Execution time: %f seconds\n", t_spent);
    
  pthread_mutex_destroy(&t_mutex);
  pthread_cond_destroy(&t_cond_bar);
  free(tids);
  printf("Main thread finished\n");
  pthread_exit(NULL);
}

float function1(float x) {
    return (1 + x);
}
float function2(float x) {
    return sqrt(1 - pow(x, 2));
}
float function3(float x) {
    return sqrt(1 + pow(x, 4));
}
float function4(float x) {
    return sin(pow(x, 2));
}
float function5(float x) {
    return cos(pow(M_E, x * -1));
}
float function6(float x) {
    return cos(pow(M_E, x * -1)) * x;
}
float function7(float x) {
    return cos(pow(M_E, x * -1)) * (0.005 * pow(x, 3) + 1);
}

float midPoint(float a, float b) {
  return (a + b) /2;
}