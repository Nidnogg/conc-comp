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
#define SIZERECTARRAY 1024
#define SPLITRECTANGLES 5   // Número de Retângulos que serão gerados apartir de um maior pelas Threads

float function(float x, int functionId);
float midPoint(float a, float b);

// Cria a estrutura de dados para armazenar os argumentos da thread
typedef struct {
   int idThread;
   int input_function_type;
} Args_t;

typedef struct {
  float area, a, b;
} Rect_t;

// Variáveis Globais
pthread_mutex_t mutex;
pthread_cond_t cond;

float totalArea = 0; //Retorno das Threads
float err;

// Variaveis Globais do Buffer
Rect_t rectsBuffer[SIZERECTARRAY];
int bufferPointer = 0;
int elementsInBuffer = 0;



// Função Executada pelas Threads
void * compute_area (void *arg) {
    Args_t *args = (Args_t *) arg;
    float areaBigRect, a, b;
    float areaSmallRects[SPLITRECTANGLES], totalAreaSmallRects, intervalStart, intervalLength;

    pthread_mutex_lock(&mutex);

    while(elementsInBuffer > 0) {
        if(elementsInBuffer > SIZERECTARRAY) {
            printf("ERROR! Buffer size limit reached.\n");
            exit(-1);
        }

        a = rectsBuffer[bufferPointer].a;
        b = rectsBuffer[bufferPointer].b;
        areaBigRect = rectsBuffer[bufferPointer].area;

        bufferPointer = (bufferPointer + 1) % SIZERECTARRAY; // "Remove" o Retangulo do Buffer
        elementsInBuffer--;
        pthread_mutex_unlock(&mutex);

        // PARTE CONCORRENTE

        totalAreaSmallRects = 0;
        intervalStart = a;
        intervalLength = (b - a) / SPLITRECTANGLES;

        for(int i = 0; i < SPLITRECTANGLES; i++) {
            areaSmallRects[i] = intervalLength * function(midPoint(intervalStart, intervalStart + intervalLength), args->input_function_type);
            totalAreaSmallRects += areaSmallRects[i];
            intervalStart += intervalLength;
        }

        if(fabs(areaBigRect - totalAreaSmallRects) > err) {
            intervalStart = a;
            pthread_mutex_lock(&mutex);
            printf("tid %d criou rect\n", args->idThread);
            for(int i = 0; i < SPLITRECTANGLES; i++) {
              bufferPointer = (bufferPointer - 1) % SIZERECTARRAY; // Sobe o ponteiro do buffer para "empilhar" o novo retângulo
              elementsInBuffer += 1; // Incrementa o contador de elementos no buffer

              // Armazena no Buffer as informações sobre cada Retângulo Menor novo:
              rectsBuffer[bufferPointer].a = intervalStart;
              intervalStart += intervalLength;
              rectsBuffer[bufferPointer].b = intervalStart;
              rectsBuffer[bufferPointer].area = areaSmallRects[i];
            }
            pthread_mutex_unlock(&mutex);
        } else { 
          printf('entei no eus tid %d\n', args->idThread);
          totalArea += areaBigRect;
        }
        pthread_mutex_lock(&mutex);
    }

  free(arg);
  pthread_exit(NULL);
}



int main(int argc, char* argv[]) {
  pthread_t tids[NTHREADS];

  Args_t *arg; //Receberá os argumentos para a thread
  float a, b; //Intervalo de integração
  int input;
  double t_start, t_end, t_spent;
 
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);

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

  // Exits if user input is incorrect
  if(input < 1 || input > 7) { 
    printf("Invalid input\n"); 
    exit(-1);
  }

  if(input == 2) {
      if((a <= -1 || b <= -1) || (a >= 1 || b >= 1)) {
          printf("Invalid interval\n");
          exit(-1);
      }
  }


  // Readies Buffer With First NTHREADS Rectangles
  elementsInBuffer = NTHREADS;
  bufferPointer = 0;
  for(int i = 0; i < NTHREADS; i++) {
    rectsBuffer[i].a = a + i * (b - a) / NTHREADS;
    rectsBuffer[i].b = rectsBuffer[i].a + (b - a) / NTHREADS;
    rectsBuffer[i].area = function(midPoint(rectsBuffer[i].a, rectsBuffer[i].b), input);
  }

  // Creates Threads
  for(int i = 0; i < NTHREADS; i++) {
    arg = malloc(sizeof(Args_t));
    if (arg == NULL) {
      printf("Failed to malloc()\n"); exit(-1);
    }
    arg->idThread = i; 
    arg->input_function_type = input;
    if (pthread_create(&tids[i], NULL, compute_area, (void *) arg)) {
      printf("Failed to pthread_create()\n"); exit(-1);
    }
  }


  // Joins Threads
  for(int i = 0; i < NTHREADS; i++) {
    if(pthread_join(tids[i], NULL)) {
      printf("Failed to pthread_create()!\n"); exit(-1);
    }
  }

  printf("Estimated area: %f\n", totalArea);
  GET_TIME(t_end);

  t_spent = t_end - t_start;
  printf("Execution time: %f seconds\n", t_spent);
    
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
 // free(tids);
  printf("Main thread finished\n");
  pthread_exit(NULL);
}



float function(float x, int functionId) {
    switch(functionId) {
        case 1:
            return (1 + x);
            break;
        case 2:
            return sqrt(1 - pow(x, 2));
            break;
        case 3:
            return sqrt(1 + pow(x, 4));
            break;
        case 4:
            return sin(pow(x, 2));
            break;
        case 5:
            return cos(pow(M_E, x * -1));
            break;
        case 6:
            return cos(pow(M_E, x * -1)) * x;
            break;
        case 7:
            return cos(pow(M_E, x * -1)) * (0.005 * pow(x, 3) + 1);
            break;
    }
    return -1;
}


float midPoint(float a, float b) {
  return (a + b) /2;
}