#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

int goto_label = 0;

// 関数名を入れる
char name[100] = {0};

// 引数名
char argName[100] = {0};

// 引数リスト 第6引数まで
char *args1[6] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
char *args4[6] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
char *args8[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_lval(Node *node) {
    if (node->kind == ND_DEREF) {
        gen(node->lhs);
        return;
    }

    if (node->kind == ND_LVAR) {
        printf("    mov rax, rbp\n");
        printf("    sub rax, %d\n", node->offset);
        printf("    push rax\n");
        return;
    } else if (node->kind == ND_GVAR) {
        strncpy(name, node->str, node->len);
        name[node->len] = '\0';
        Type *t = get_type(node);
        if (t->ty == INT) {
            // TODO: INT以外の実装
            printf("    push offset %s\n", name);
        }

        return;
    }

    error("代入の左辺値が変数ではありません");
}

void gen(Node *node) {
    switch (node->kind) {
        case ND_IF:
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je  .Lend%d\n", goto_label);
            gen(node->then);
            printf(".Lend%d:\n", goto_label);
            goto_label++;
            return;
        case ND_IF_ELSE:
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je  .Lelse%d\n", goto_label);
            gen(node->then);
            printf("   jmp .Lend%d\n", goto_label);
            printf(".Lelse%d:\n", goto_label);
            gen(node->els);
            printf(".Lend%d:\n", goto_label);
            goto_label++;
            return;
        case ND_WHILE:
            printf(".Lbegin%d:\n", goto_label);
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je  .Lend%d\n", goto_label);
            gen(node->then);
            printf("    jmp .Lbegin%d\n", goto_label);
            printf(".Lend%d:\n", goto_label);
            goto_label++;
            return;
        case ND_FOR:
            gen(node->ini);
            printf(".Lbegin%d:\n", goto_label);
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je  .Lend%d\n", goto_label);
            gen(node->then);
            gen(node->inc);
            printf("    jmp .Lbegin%d\n", goto_label);
            printf(".Lend%d:\n", goto_label);
            goto_label++;
            return;
        case ND_BLOCK:
            while (node->next != NULL) {
                gen(node->next);
                node = node->next;
            }
            return;
        case ND_FUNC_DEF: {
            // 関数プロローグ
            strncpy(name, node->str, node->len);
            name[node->len] = '\0';
            printf(".global %s\n", name);
            printf("%s:\n", name);

            // rbp: ベースレジスタ
            printf("    push rbp\n");
            printf("    mov rbp, rsp\n");

            int argCount = 0;
            Node *next = node->argNext;
            while (next != NULL) {
                argCount++;
                next = next->argNext;
            }

            if (locals[cur_func]) {
                int offset = locals[cur_func]->offset;
                printf("    sub rsp, %d\n", offset);
            }

            int i = 0;
            Node *n = node->argNext;
            while (n != NULL) {
                if (n->size == CHAR_SIZE) {
                    printf("    mov [rbp-%d], %s\n", n->offset, args1[i]);
                } else if (n->size == INT_SIZE) {
                    printf("    mov [rbp-%d], %s\n", n->offset, args4[i]);
                } else if (n->size == PTR_SIZE) {
                    printf("    mov [rbp-%d], %s\n", n->offset, args8[i]);
                }
                n = n->argNext;
                i++;
            }

            while (node->next != NULL) {
                gen(node->next);
                node = node->next;
            }

            return;
        }
        case ND_FUNC_CALL: {
            // C言語だとcaseの中で変数を宣言できないので、ブロックを使う
            // https://www.chihayafuru.jp/tech/index.php/archives/2990
            int argNum = 0;

            strncpy(name, node->str, node->len);
            name[node->len] = '\0';

            while (node->argNext != NULL) {
                gen(node->argNext);
                printf("    pop %s\n", args8[argNum]);
                argNum++;
                node = node->argNext;
            }

            // rspの値をチェックして、16の倍数にする（スタックは下に成長するので-8）
            printf("    mov rax, rsp\n");

            // 15とのAND => 下位4ビットのAND
            printf("    and rax, 15\n");

            // 下位4ビットが0でない => 16で割り切れていない
            printf("    jnz .L.call.%d\n", goto_label);

            printf("    mov rax, 0\n");
            printf("    call %s\n", name);
            printf("    jmp .L.end.%d\n", goto_label);
            printf(".L.call.%d:\n", goto_label);
            printf("    sub rsp, 8\n");
            printf("    mov rax, 0\n");
            printf("    call %s\n", name);
            // +8して戻す
            printf("    add rsp, 8\n");
            printf(".L.end.%d:\n", goto_label);
            printf("    push rax\n");

            goto_label++;
            return;
        }
        case ND_RETURN:
            gen(node->lhs);
            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return;
        case ND_NUM:
            // printf("NUM: %d\n\n", node->val);
            printf("    push %d\n", node->val);
            return;
        case ND_LVAR:
        case ND_GVAR: {
            if (node->kind == ND_GVAR && node->is_define) {
                strncpy(name, node->str, node->len);
                name[node->len] = '\0';
                printf("%s:\n", name);
                printf("    .zero %d\n", node->offset);
                return;
            }

            gen_lval(node);
            Type *t = get_type(node);
            if (t != NULL && t->ty == ARRAY) {
                return;
            }

            if (t != NULL && t->ty == CHAR) {
                printf("    pop rax\n");
                printf("    movsx rax, BYTE PTR [rax]\n");
                printf("    push rax\n");
                return;
            }

            if (t != NULL && t->ty == INT) {
                printf("    pop rax\n");
                printf("    movsx rax, DWORD PTR [rax]\n");
                printf("    push rax\n");
                return;
            }

            // PTR
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
            return;
        }
        case ND_STRING:
            printf("    push offset .LC%d\n", node->string->index);
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);

            Type *t = get_type(node);

            if (t && t->ty == CHAR) {
                printf("    pop rdi\n");
                printf("    pop rax\n");
                printf("    mov [rax], dil\n");
                printf("    push rdi\n");
                return;
            }

            if (t && t->ty == INT) {
                printf("    pop rdi\n");
                printf("    pop rax\n");
                printf("    mov [rax], edi\n");  // int => 32bit
                printf("    push rdi\n");
                return;
            }

            // PTR
            printf("    pop rdi\n");
            printf("    pop rax\n");
            printf("    mov [rax], rdi\n");
            printf("    push rdi\n");
            return;
        case ND_ADDR:
            gen_lval(node->lhs);
            return;
        case ND_DEREF:
            gen(node->lhs);
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
            return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            printf("    sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("    imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            break;
        case ND_EQ:
            printf("    cmp rax, rdi\n");
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_NE:
            printf("    cmp rax, rdi\n");
            printf("    setne al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LT:
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LTE:
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
            break;
    }

    printf("    push rax\n");
}