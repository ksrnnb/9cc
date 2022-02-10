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

    // 静的変数のうちプログラム開始時に0で初期化されるもの
    // => .bssのセクションが終わるまでグローバル変数
    printf(".bss\n");

    for (int i = 0; code[i]; i++) {
        if (code[i]->kind == ND_GVAR) {
            gen(code[i]);
        }
    }

    // .textの下から機械語にされる実行文
    printf(".text\n");
    cur_func = 0;
    for (int i = 0; code[i]; i++) {
        if (code[i]->kind == ND_FUNC_DEF) {
            cur_func++;
            gen(code[i]);
        }
    }

    exit(EXIT_SUCCESS);
}