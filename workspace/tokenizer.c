#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

// 入力プログラム
char *user_input;

// 現在みているトークン
Token *token;

LVar *locals[100];
int cur_func;

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

Token *consume_string() {
    if (token->kind != TK_STRING) {
        return NULL;
    }

    Token *tok = token;
    token = token->next;

    return tok;
}

TypeName consume_type() {
    if (isExpectedSymbol("int")) {
        token = token->next;
        return INT;
    }

    if (isExpectedSymbol("char")) {
        token = token->next;
        return CHAR;
    }

    return -1;
}

Token *consume_ident() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }

    Token *retToken = token;
    token = token->next;
    return retToken;
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

// 英数字か、アンダースコアのいずれか
bool is_alnum(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || (c == '_');
}

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

        // returnの次が英数字、アンダースコアでないこと
        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RESERVED, cur, p, 6);
            p += 6;
            continue;
        }

        if (strncmp(p, "sizeof", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RESERVED, cur, p, 6);
            p += 6;
            continue;
        }

        if (strncmp(p, "int", 3) == 0 && !is_alnum(p[3])) {
            cur = new_token(TK_RESERVED, cur, p, 3);
            p += 3;
            continue;
        }

        if (strncmp(p, "char", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_RESERVED, cur, p, 4);
            p += 4;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_RESERVED, cur, p, 4);
            p += 4;
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_RESERVED, cur, p, 5);
            p += 5;
            continue;
        }

        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            cur = new_token(TK_RESERVED, cur, p, 3);
            p += 3;
            continue;
        }

        // 現段階では、記号や数字は識別子に含めない
        if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) {
            char *pp = p;
            int len = 0;

            while (is_alnum(*pp)) {
                pp++;
                len++;
            }

            cur = new_token(TK_IDENT, cur, p, len);
            p = pp;
            continue;
        }

        if (starts_with(p, ">=") || starts_with(p, "<=") ||
            starts_with(p, "==") || starts_with(p, "!=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/()<>=;{},&[]", *p)) {
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

        if (*p == '"') {
            p++;
            char *c = p;
            while (*c != '"') {
                c++;
            }
            int len = c - p;
            cur = new_token(TK_STRING, cur, p, len);
            p = c + 1;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

// 変数を名前で検索する
LVar *find_lvar(Token *tok) {
    for (LVar *var = locals[cur_func]; var; var = var->next) {
        // memcmpは完全一致なら0を返す
        if (var->len == tok->len &&
            memcmp(tok->str, var->name, var->len) == 0) {
            return var;
        }
    }

    return NULL;
}

// 変数を名前で検索する
GVar *find_gvar(Token *tok) {
    for (GVar *var = globals; var; var = var->next) {
        // memcmpは完全一致なら0を返す
        if (var->len == tok->len &&
            memcmp(tok->str, var->name, var->len) == 0) {
            return var;
        }
    }

    return NULL;
}