#include <stdio.h>

int foo() {
    printf("OK\n");
    return 1;
}

int bar(int x, int y) {
    printf("%d\n", x + y);
    return x + y;
}

int bar3(int x, int y, int z) {
    printf("%d\n", x + y + z);
    return x + y + z;
}