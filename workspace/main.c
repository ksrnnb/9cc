#include <stdio.h>
#include <stdlib.h>

#include "9cc.h"

int cur_func;
LVar *locals[100];
GVar *globals;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s: argument is invalid\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    user_input = argv[1];
    token = tokenize();
    program();

    // プロローグ
    printf(".intel_syntax noprefix\n");

    cur_func = 0;
    for (int i = 0; code[i]; i++) {
        cur_func++;
        gen(code[i]);
    }

    exit(EXIT_SUCCESS);
}