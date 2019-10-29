/* Disciplina: Computacao Concorrente */
/* Grupo: Henrique Vermelho de Toledo, João Pedro Lopes Murtinho */
/* Prof.: Silvana Rossetto */
/* Trabalho de Implementação */
/* Codigo: Solução Sequencial do Método de Integração Numérica Retangular usando Quadratura Adaptativa */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "timer.h"

// Protótipos de Funções 
float function(float x, int functionId);
float quad(float a, float b, float err, int input_function_type);
float midPoint(float a, float b);
int main(int argc, char* argv[]) {

  float a, b; // Intervalo de integração
  float err;  // Erro máximo
  float result; // Resultado final da área da função abaixo do intervalo dado
  int input_function_type; // Tipo de função, escolhida pelo usuário
  double t_start, t_end, t_spent; // Funções para medição de tempo
 

  // Checagem dos parametros passados na linha de comando
  if(argc < 4) {
    fprintf(stderr, "Parameters: %s <start of interval(a)> <end of interval(b)> <max error(err)>  \n", argv[0]);
    return 1;
  }

  // Conversão dos argumentos para float
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

  scanf("%d", &input_function_type);

  // Fecha programa se input é inconsistente com as opçõees
  if(input_function_type < 1 || input_function_type > 7) { 
    printf("Invalid input_function_type\n"); 
    exit(-1);
  }

  // Para o caso da função 2, checa se input respeita o intervalo dado
  if(input_function_type == 2){
      if((a <= -1 || b <= -1) || (a >= 1 || b >= 1)){
          printf("Invalid interval\n");
          exit(-1);
      }
  }
  GET_TIME(t_start);

  // Computa recursivamente a integral
  result = quad(a, b, err, input_function_type);
  printf("Estimated area: %f\n", result);

  GET_TIME(t_end);
  t_spent = t_end - t_start;
  printf("Execution time: %.4fs\n", t_spent);
    
  return 0;
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

// Nessa função, input_function_type determina qual caso teste (qual função a ser integrada) será realizado
float quad(float a, float b, float err, int input_function_type) {

  float current_area_large, current_area_small1, current_area_small2; //current_area_large é área do retângulo maior, small1 é a do triangulo
  float bigRectX = midPoint(a, b); // x do ponto médio inicial
  float smallRectX1 = midPoint(a, bigRectX); // ponto médio do retângulo small1
  float smallRectX2 = midPoint(bigRectX, b); // ponto médio do retângulo small2
  
  current_area_large = (b - a) * function(bigRectX, input_function_type);
  current_area_small1 = (bigRectX - a) * function(smallRectX1, input_function_type);
  current_area_small2 = (b - bigRectX) * function(smallRectX2, input_function_type);

  // Subtração para ver se a aproximação é boa
  if(fabs(current_area_large - (current_area_small1 + current_area_small2)) < err ) {
    return current_area_large;
  } else {
    float area = quad(a, bigRectX, err, input_function_type) + quad(bigRectX, b, err, input_function_type);
    return area;
  }
  
  
}

float midPoint(float a, float b) {
  return (a + b) / 2;
}
