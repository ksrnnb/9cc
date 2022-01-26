#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    TK_RESERVED,  // 記号
    TK_NUM,       // 数字
    TK_EOF,       // 終端
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};

// 現在みているトークン
Token *token;

// 期待している記号かどうか
bool isExpectedSymbol(char expected) {
    return token->kind == TK_RESERVED && token->str[0] == expected;
}

// エラー出力
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

// 期待している記号である場合、トークンを1つ読み進んでtrueを返す
bool consume(char op) {
    if (!isExpectedSymbol(op)) return false;

    token = token->next;
    return true;
}

// 期待している記号であるときは、トークンを1つ読み進める
void expect(char op) {
    if (!isExpectedSymbol(op)) error("'%c'ではありません", op);

    token = token->next;
}

// 数字の場合はトークンを1つ進めて、数字を返す
int expect_number() {
    if (token->kind != TK_NUM) error("数ではありません");

    int val = token->val;
    token = token->next;

    return val;
}

// 終端かどうか
bool at_eof() { return token->kind == TK_EOF; }

// トークンの新規作成
// 新しくトークンを作成して、カーソルの次のトークンに新規作成したトークンを指定
Token *new_token(TokenKind kind, Token *cur, char *str) {
    // calloc: 確保した領域の全ビットを0で初期化
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// 受け取った文字列からトークン化する
// トークンは連結リストとなる
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s: argument is invalid\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("    mov rax, %d\n", expect_number());

    while (!at_eof()) {
        if (consume('+')) {
            printf("    add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    printf("    ret\n");
    exit(EXIT_SUCCESS);
}