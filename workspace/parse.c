#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

// 入力プログラム
char *user_input;

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
    int len;
};

// 現在みているトークン
Token *token;

// エラー箇所を出力
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");  // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

// haystackがneedleで始まるかどうか
bool starts_with(char *haystack, char *needle) {
    return memcmp(haystack, needle, strlen(needle)) == 0;
}

// 期待している記号かどうか
bool isExpectedSymbol(char *expected) {
    return token->kind == TK_RESERVED && strlen(expected) == token->len &&
           memcmp(token->str, expected, token->len) == 0;
}

// 期待している記号である場合、トークンを1つ読み進んでtrueを返す
bool consume(char *op) {
    if (!isExpectedSymbol(op)) return false;

    token = token->next;
    return true;
}

// 期待している記号であるときは、トークンを1つ読み進める
void expect(char *op) {
    if (!isExpectedSymbol(op)) error_at(token->str, "'%c'ではありません", op);

    token = token->next;
}

// 数字の場合はトークンを1つ進めて、数字を返す
int expect_number() {
    if (token->kind != TK_NUM) error_at(token->str, "数ではありません");

    int val = token->val;
    token = token->next;

    return val;
}

// 終端かどうか
bool at_eof() { return token->kind == TK_EOF; }

// トークンの新規作成
// 新しくトークンを作成して、カーソルの次のトークンに新規作成したトークンを指定
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    // calloc: 確保した領域の全ビットを0で初期化
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

// 受け取った文字列からトークン化する
// トークンは連結リストとなる
Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (starts_with(p, ">=") || starts_with(p, "<=") ||
            starts_with(p, "==") || starts_with(p, "!=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/()<>", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            // 有効でない文字が現れたら、そこで変換が終了する。
            cur->val = strtol(p, &p, 10);
            // はじめの文字の位置から、数字分だけ進んだ位置を引く
            // =>トークンの長さがわかる
            cur->len = q - p;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}