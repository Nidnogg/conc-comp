/* Disciplina: Computacao Concorrente */
/* Grupo: Henrique Vermelho de Toledo, João Pedro Lopes Murtinho */
/* Prof.: Silvana Rossetto */
/* Trabalho de Implementação */
/* Codigo: Solução Concorrente do Método de Integração Numérica Retangular usando Quadratura Adaptativa */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time      .h>

float quad (float a, float b, float err, int input);
float midPoint (float a, float b);
int main(int argc, char* argv[]) {

  float a, b; //Intervalo de integração
  float err;
  float result;
  int input; 
  clock_t begin = clock();


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
  if(input < 1 || input > 7) { 
    printf("Invalid input\n"); 
    exit(-1);
  }

  result = quad(a, b, err, input);
  printf("Estimated area: %f\n", result);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("Execution time: %.4f\n", time_spent);
    
  return 0;
}

float quad (float a, float b, float err, int input) {

  float current_area_large, current_area_small1, current_area_small2;
  float bigRectX = midPoint(a, b); //initial mid point
  float smallRectX1 = midPoint(a, bigRectX);
  float smallRectX2 = midPoint(bigRectX, b);

  switch(input) {
    case 1: // 1 + x
      current_area_large = (b - a) * (1 + bigRectX);
      current_area_small1 = (bigRectX - a) * (1 + smallRectX1);
      current_area_small2 = (b - bigRectX) * (1 + smallRectX2);

      if(abs(current_area_large - (current_area_small1 + current_area_small2)) < err ) {
        return current_area_large;
      } else {
        float area = quad(a, bigRectX, err, input) + quad(bigRectX, b, err, input);
        return area;
      }
      break;

    default:
      printf("Invalid input\n");
      exit(-1);
      break;
  }
}

float midPoint (float a, float b) {
  return (a + b)/2;
}
