#include <stdlib.h>
#include <stdio.h>

void fib(int **fib_sequence, int n){
    *fib_sequence = malloc(sizeof(int) * n);
    int *arr = *fib_sequence;

    arr[0] = 0;
    arr[1] = 1;

    for(int i = 2; i < n; i++){
        arr[i] = arr[i-1] + arr[i-2];
    }
}


int main(int argc, char **argv) {
    /* do not change this main function */
    int count = strtol(argv[1], NULL, 10);
    int *fib_sequence;

    fib(&fib_sequence, count);
    for (int i = 0; i < count; i++) {
        printf("%d ", fib_sequence[i]);
    }
    free(fib_sequence);
    return 0;
}
