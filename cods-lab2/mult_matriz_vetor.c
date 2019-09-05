/* Disciplina: Computacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 2 */
/* Codigo: Multiplica uma matriz por um vetor */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

//variaveis globais
//float *matA; //matriz de entrada
//float *vetX; //vetor de entrada
//float *vetB; //vetor de saida

//funcao que multiplica matriz por vetor (A * X = B)
//entrada: matriz de entrada, vetor de entrada, vetor de saida, dimensoes da matriz
//requisito 1: o numero de colunas da matriz eh igual ao numero de elementos do vetor de entrada
//requisito 2: o numero de linhas da matriz eh igual ao numero de elementos do vetor de saida
void multiplicaMatrizVetor(const float *a, const float *x, float *b, int linhas, int colunas) {
   int i, j;
   for (i=0; i<linhas; i++) {
      b[i] = 0;
      for (j=0; j<colunas; j++) {
         b[i] += a[i*colunas+j] * x[j];
      }
   }
}

//funcao que aloca espaco para uma matriz e preenche seus valores
//entrada: matriz de entrada, dimensoes da matriz
//saida: retorna 1 se a matriz foi preenchida com sucesso e 0 caso contrario
int preencheMatriz(float **mat, int linhas, int colunas, FILE *arq) {
   int i, j;
   //aloca espaco de memoria para a matriz
   *mat = (float*) malloc(sizeof(float) * linhas * colunas);
   if (*mat == NULL) return 0;
   //preenche o vetor
   for (i=0; i<linhas; i++) {
      for (j=0; j<colunas; j++) {
         fscanf(arq, "%f", (*mat) + (i*colunas+j));
      }
   }
   return 1;
}

//funcao que imprime uma matriz
//entrada: matriz de entrada, dimensoes da matriz
//saida: matriz impressa na tela
void imprimeMatriz(float *mat, int linhas, int colunas, FILE *arq) {
   int i, j;
   for (i=0; i<linhas; i++) {
      for (j=0; j<colunas; j++) {
         fprintf(arq, "%.1f ", mat[i*colunas+j]);
      }
      fprintf(arq, "\n");
   }
}

//funcao que aloca espaco para um vetor e preenche seus valores
//entrada: vetor de entrada, dimensao do vetor
//saida: retorna 1 se o vetor foi preenchido com sucesso e 0 caso contrario
int preencheVetor(float **vet, int dim, FILE *arq) {
   int i;
   //aloca espaco de memoria para o vetor
   *vet = (float*) malloc(sizeof(float) * dim);
   if (*vet == NULL) return 0;
   //preenche o vetor
   for (i=0; i<dim; i++) {
       //*( (*vet)+i ) = 1.0;
       fscanf(arq, "%f", (*vet) + i);
   }
   return 1;
}

//funcao que imprime um vetor
//entrada: vetor de entrada, dimensao do vetor
//saida: vetor impresso na tela
void imprimeVetor(float *vet, int dim, FILE *arq) {
   int i;
   for (i=0; i<dim; i++) {
      fprintf(arq, "%.1f ", vet[i]);
   }
   fprintf(arq, "\n");
}

//funcao principal
int main(int argc, char *argv[]) {
   float *matA; //matriz de entrada
   float *vetX; //vetor de entrada
   float *vetB; //vetor de saida
   double inicio, fim, delta1, delta2, delta3;
   FILE *arqA, *arqX, *arqB, *tempos; //arquivos dos dados de entrada e saida
   int linhas, colunas; //dimensoes da matriz de entrada
   int dim; //dimensao do vetor de entrada


   //le e valida os parametros de entrada
   //o arquivo da matriz de entrada deve conter na primeira linha as dimensoes da matriz (linha coluna) seguido dos elementos da matriz separados por espaco
   //o arquivo do vetor de entrada deve conter na primeira linha a dimensao do vetor seguido dos elementos separados por espaco


   //le tempo de inicio
   GET_TIME(inicio);

   
   if(argc < 4) {
      fprintf(stderr, "Digite: %s <arquivo matriz A> <arquivo vetor X> <arquivo vetor B>.\n", argv[0]);
      return 1;
   }
   //abre o arquivo da matriz de entrada
   arqA = fopen(argv[1], "r");
   if(arqA == NULL) {
      fprintf(stderr, "Erro ao abrir o arquivo da matriz de entrada.\n");
      return 1;
   }
   //le as dimensoes da matriz de entrada
   fscanf(arqA, "%d", &linhas);
   fscanf(arqA, "%d", &colunas);

   //abre o arquivo do vetor de entrada
   arqX = fopen(argv[2], "r");
   if(arqX == NULL) {
      fprintf(stderr, "Erro ao abrir o arquivo do vetor de entrada.\n");
      return 1;;
   }
   //le a dimensao do vetor de entrada
   fscanf(arqX, "%d", &dim);

   //valida as dimensoes da matriz e vetor de entrada
   if(colunas != dim) {
      fprintf(stderr, "Erro: as dimensoes da matriz e do vetor de entrada nao sao compativeis.\n");
      return 1;
   }

   //abre o arquivo do vetor de saida
   arqB = fopen(argv[3], "w");
   //arqB = fopen("data/B%d.txt")
   if(arqB == NULL) {
      fprintf(stderr, "Erro ao abrir o arquivo do vetor de saida.\n");
      return 1;
   }

   //aloca e preenche a matriz de entrada
   if(preencheMatriz(&matA, linhas, colunas, arqA) == 0) {
      fprintf(stderr, "Erro de preenchimento da matriz de entrada\n");
      return 1;
   }
   //aloca e preenche o vetor de entrada
   if(preencheVetor(&vetX, dim, arqX) == 0) {
      fprintf(stderr, "Erro de preenchimento do vetor de entrada\n");
      return 1;
   }
   //aloca o vetor de saida
   vetB = (float*) malloc(sizeof(float) * linhas);
   if(vetB==NULL) {
      fprintf(stderr, "Erro de alocacao do vetor de saida\n");
      return 1;
   }


   //le tempo de fim e em seguida computa o delta1, tempo pra inicialização
   GET_TIME(fim);
   delta1 = fim - inicio;
   /*
   //imprime a matriz de entrada
   printf("Matriz de entrada:\n");
   imprimeMatriz(matA, linhas, colunas, stdout);
   //imprime o vetor de entrada
   printf("Vetor de entrada:\n");
   imprimeVetor(vetX, colunas, stdout);
   */

   //multiplica a matriz de entrada pelo vetor de entrada

   GET_TIME(inicio);
   multiplicaMatrizVetor(matA, vetX, vetB, linhas, colunas);
   
   //salva o vetor de saida no arquivo de saida
   imprimeVetor(vetB, linhas, arqB);

   //le tempo de fim e em seguida computa o delta1, tempo para a parte de multip
   GET_TIME(fim);
   delta2 = fim - inicio;

   GET_TIME(inicio);
   //libera os espacos de memoria alocados
   free(matA);   
   free(vetX);   
   free(vetB);   
   GET_TIME(fim);
   delta3 = fim - inicio;

   //exibe os tempos gastos em cada parte do programa 
   
   tempos = fopen("./data/T1024.txt", "w");
   if(tempos == NULL) {
      fprintf(stderr, "Erro em arquivo de armazenamento dos tempos\n");
      return 1;
   }

   fprintf(stdout, "Tempo inicializacoes: %.8lf\n", delta1);
   fprintf(stdout, "Tempo multiplica matriz com %d threads: %.8lf\n", 1, delta2);
   fprintf(stdout, "Tempo finalizacoes: %.8lf\n", delta3);

   
   return 0;
}

