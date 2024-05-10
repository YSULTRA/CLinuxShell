#include <stdio.h>
#include <stdlib.h>

//Helping Code For Fib.c
void fib(int n) {
    printf("Number Entered: %d\n", n);
    int a = 0, b = 1, next;
    printf("Fibonacci Sequence: %d, %d, ", a, b);
    for (int i = 2; i < n; i++) {
        next = a + b;
        printf("%d, ", next);
        a = b;
        b = next;
    }

    printf("\n");
}

int main(int argc, char *argv[]) {

    int n = atoi(argv[1]);
    fib(n);
    return 0; }