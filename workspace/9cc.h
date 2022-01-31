#include <stdbool.h>

typedef struct Node Node;

typedef struct Token Token;

typedef enum {
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_EQ,      // ==
    ND_NE,      // !=
    ND_LT,      // <
    ND_LTE,     // <=
    ND_ASSIGN,  // =
    ND_LVAR,    // ローカル変数
    ND_NUM,     // 整数
} NodeKind;

typedef enum {
    TK_RESERVED,  // 記号
    TK_IDENT,     // 識別子
    TK_NUM,       // 数字
    TK_EOF,       // 終端
} TokenKind;

struct Node {
    NodeKind kind;  // ノードの型
    Node *lhs;      // 左辺
    Node *rhs;      // 右辺
    int val;        // kindがND_NUMの場合に使用
    int offset;     // kindかND_LVARの場合に使用
};

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

// 入力プログラム
extern char *user_input;

// 現在みているトークン
extern Token *token;

// トークン生成
Token *tokenize();

// 再帰下降構文解析
Node *expr();

// コード生成
void gen(Node *node);

// 期待している記号であるときは、トークンを1つ読み進める
void expect(char *op);

// 数字の場合はトークンを1つ進めて、数字を返す
int expect_number();

// 期待している記号である場合、トークンを1つ読み進んでtrueを返す
bool consume(char *op);

// 識別子の場合、現在のトークンを返して、トークンを1つ進める
Token *consume_ident();

// 終端かどうか
bool at_eof();