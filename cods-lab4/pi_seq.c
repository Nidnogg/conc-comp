#include <stdio.h> 
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]) {
  
  printf("%lli\n", atoll(argv[1]));
  double sum = 1.0;
  double aux = 0.0;
  long long int n = atoll(argv[1]);
  for(long long int i = 0; i < n; i++) {
    if(i % 2 == 0) {
      sum -= 1.0/(aux + 3.0);
      printf("IF i %lli i mod 2 %lli %lf\n", i, i %2,  (aux + 3.0));
      aux += 2.0;
    } else {
      printf("ELSE i %lli i mod 2 %lli %lf\n", i, i %2,  (aux + 3.0));
      sum += 1.0/(aux + 3.0);
      aux += 2.0;
    }
  }

  sum = 4.0 * sum;
  printf("M_PI %lf pi %lf\n", M_PI, sum);

  return 0;
}
