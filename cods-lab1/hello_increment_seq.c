#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 100000

int main(void) {
    int i;
    int *elements;
    clock_t begin = clock();

    elements = malloc(sizeof(int) * N);
    if(elements == NULL) {
        printf("Failed to malloc!\n"); exit(-1);
    }

    printf("Elements before increment:\n");
    for(i = 0; i < N; i++) {
        *(elements + i) = i;
        printf("%d\n", *(elements + i));

    }

    printf("Elements after increment:\n");
    for (i = 0; i < N; i++) {
        *(elements + i) += 1;
        printf("%d\n", *(elements + i));
    }


    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Execution time: %.4f\n", time_spent);

    return 0;

}