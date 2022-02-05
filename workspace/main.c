#include <stdio.h>
#include <stdlib.h>

#include "9cc.h"

int cur_func;
LVar *locals[100];

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
    printf(".globl main\n");

    // rbp: ベースレジスタ
    // 208 => 8 * 26
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");

    cur_func = 0;
    for (int i = 0; code[i]; i++) {
        cur_func++;
        gen(code[i]);
    }

    exit(EXIT_SUCCESS);
}