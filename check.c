#include <stdio.h>

int n = 1;

void check(int x) {
    if (!x) printf("not ");
    printf("ok %d\n", n++);
}
