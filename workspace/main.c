#include <stdio.h>
#include <stdlib.h>

#include "9cc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s: argument is invalid\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    user_input = argv[1];
    token = tokenize();
    program();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // rbp: ベースレジスタ
    // 208 => 8 * 26
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    for (int i = 0; code[i]; i++) {
        gen(code[i]);

        printf("    pop rax\n");
    }

    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");

    exit(EXIT_SUCCESS);
}