#include <stdbool.h>

typedef struct Node Node;

typedef struct Token Token;

typedef struct LVar LVar;

typedef enum {
    ND_ADD,      // +
    ND_SUB,      // -
    ND_MUL,      // *
    ND_DIV,      // /
    ND_EQ,       // ==
    ND_NE,       // !=
    ND_LT,       // <
    ND_LTE,      // <=
    ND_ASSIGN,   // =
    ND_LVAR,     // ローカル変数
    ND_NUM,      // 整数
    ND_RETURN,   // return
    ND_IF,       // if
    ND_IF_ELSE,  // if-else
    ND_WHILE,    // while
    ND_FOR,      // for
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
    Node *cond;     // if/while/for文で利用
    Node *then;     // if/while文で利用
    Node *els;      // if文で利用
    Node *ini;      // for文
    Node *inc;      // for文
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

// ローカル変数の型
struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;  // RBPからのオフセット
};

// ローカル変数
LVar *locals;

// 入力プログラム
extern char *user_input;

// 現在みているトークン
extern Token *token;

// 解析したstatementを格納する
extern Node *code[100];

// トークン生成
Token *tokenize();

// 構文解析
void program();

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

// 変数を名前で検索する
LVar *find_lvar(Token *tok);