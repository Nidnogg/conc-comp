/* Disciplina: Computacao Concorrente */
/* Grupo: Henrique Vermelho de Toledo, João Pedro Lopes Murtinho */
/* Prof.: Silvana Rossetto */
/* Trabalho de Implementação */
/* Codigo: Solução Concorrente do Método de Integração Numérica Retangular usando Quadratura Adaptativa */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "timer.h"

float quad (float a, float b, float err, int input_function_type);
float midPoint (float a, float b);
int main(int argc, char* argv[]) {

  float a, b; //Intervalo de integração
  float err;
  float result;
  int input_function_type;
  double t_start, t_end, t_spent;
 
  GET_TIME(t_start);

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

  scanf("%d", &input_function_type);

  // Quits if user input_function_type is incorrect
  if(input_function_type < 1 || input_function_type > 7) { 
    printf("Invalid input_function_type\n"); 
    exit(-1);
  }

  if(input_function_type == 2){
      if((a <= -1 || b <= -1) || (a >= 1 || b >= 1)){
          printf("Invalid interval\n");
          exit(-1);
      }
  }

  result = quad(a, b, err, input_function_type);
  printf("Estimated area: %f\n", result);

  GET_TIME(t_end);

  t_spent = t_end - t_start;
  printf("Execution time: %f seconds\n", t_spent);
    
  return 0;
}

float quad (float a, float b, float err, int input_function_type) {

  float current_area_large, current_area_small1, current_area_small2;
  float bigRectX = midPoint(a, b); //initial mid point
  float smallRectX1 = midPoint(a, bigRectX);
  float smallRectX2 = midPoint(bigRectX, b);

  switch(input_function_type) {
    case 1: // 1 + x
      current_area_large = (b - a) * (1 + bigRectX);
      current_area_small1 = (bigRectX - a) * (1 + smallRectX1);
      current_area_small2 = (b - bigRectX) * (1 + smallRectX2);

      if(fabs(current_area_large - (current_area_small1 + current_area_small2)) < err ) {
        return current_area_large;
      } else {
        float area = quad(a, bigRectX, err, input_function_type) + quad(bigRectX, b, err, input_function_type);
        return area;
      }
      break;

    case 2: //sqrt(1-x^2)
      current_area_large = (b - a) * sqrt(1 - pow(bigRectX, 2));
      current_area_small1 = (bigRectX - a) * sqrt(1 - pow(smallRectX1, 2));
      current_area_small2 = (b - bigRectX) * sqrt(1 - pow(smallRectX2, 2));

      if(fabs(current_area_large - (current_area_small1 + current_area_small2)) < err ) {
        return current_area_large;
      } else {
        float area = quad(a, bigRectX, err, input_function_type) + quad(bigRectX, b, err, input_function_type);
        return area;
      }
      break;

    case 3: //sqrt(1 + x^4)
      current_area_large = (b - a) * sqrt(1 + pow(bigRectX, 4));
      current_area_small1 = (bigRectX - a) * sqrt(1 + pow(smallRectX1, 4));
      current_area_small2 = (b - bigRectX) * sqrt(1 + pow(smallRectX2, 4));

      if(fabs(current_area_large - (current_area_small1 + current_area_small2)) < err ) {
        return current_area_large;
      } else {
        float area = quad(a, bigRectX, err, input_function_type) + quad(bigRectX, b, err, input_function_type);
        return area;
      }
      break;    

    case 4: //sin(x^2)
      current_area_large = (b - a) * sin(pow(bigRectX, 2));
      current_area_small1 = (bigRectX - a) * sin(pow(smallRectX1, 2));
      current_area_small2 = (b - bigRectX) * sin(pow(smallRectX2, 2));

      if(fabs(current_area_large - (current_area_small1 + current_area_small2)) < err ) {
        return current_area_large;
      } else {
        float area = quad(a, bigRectX, err, input_function_type) + quad(bigRectX, b, err, input_function_type);
        return area;
      }
      break;

    case 5: //cos(e^-x)
      current_area_large = (b - a) * cos(pow(M_E, bigRectX * -1));
      current_area_small1 = (bigRectX - a) * cos(pow(M_E, smallRectX1 * -1));
      current_area_small2 = (b - bigRectX) * cos(pow(M_E, smallRectX2 * -1));

      if(fabs(current_area_large - (current_area_small1 + current_area_small2)) < err ) {
        return current_area_large;
      } else {
        float area = quad(a, bigRectX, err, input_function_type) + quad(bigRectX, b, err, input_function_type);
        return area;
      }
      break;        

    case 6: //cos(e^-x) * x
      current_area_large = (b - a) * cos(pow(M_E, bigRectX * -1)) * bigRectX;
      current_area_small1 = (bigRectX - a) * cos(pow(M_E, smallRectX1 * -1)) * smallRectX1;
      current_area_small2 = (b - bigRectX) * cos(pow(M_E, smallRectX2 * -1)) * smallRectX2;

      if(fabs(current_area_large - (current_area_small1 + current_area_small2)) < err ) {
        return current_area_large;
      } else {
        float area = quad(a, bigRectX, err, input_function_type) + quad(bigRectX, b, err, input_function_type);
        return area;
      }
      break;
      
    case 7: //cos(e^-x) * (0.005 * x^3 + 1)
      current_area_large = (b - a) * cos(pow(M_E, bigRectX * -1)) * (0.005 * pow(bigRectX, 3) + 1);
      current_area_small1 = (bigRectX - a) * cos(pow(M_E, smallRectX1 * -1)) * (0.005 * pow(smallRectX1, 3) + 1);
      current_area_small2 = (b - bigRectX) * cos(pow(M_E, smallRectX2 * -1)) * (0.005 * pow(smallRectX2, 3) + 1);

      if(fabs(current_area_large - (current_area_small1 + current_area_small2)) < err ) {
        return current_area_large;
      } else {
        float area = quad(a, bigRectX, err, input_function_type) + quad(bigRectX, b, err, input_function_type);
        return area;
      }
      break;

    default:
      printf("Invalid input_function_type\n");
      exit(-1);
      break;
  }
}

float midPoint (float a, float b) {
  return (a + b)/2;
}
