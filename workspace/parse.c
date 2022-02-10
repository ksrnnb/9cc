#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

void program();
Node *func();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *function_node(Token *tok);
Type *get_var_type();
Node *lvar(Token *tok, Node *node);
void define_variable(Token *tok, Node *node, Type *type);
Node *define_gvar(Token *tok, Type *type);
Node *dispatch_define_gvar(Token *tok, Node *node, Type *type);

// 解析したstatementを格納する
Node *code[100];

// 現在みているトークン
Token *token;

// ローカル変数用
LVar *locals[100];
int cur_func = 0;

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;

    return node;
}

void program() {
    int i = 0;
    while (!at_eof()) {
        code[i++] = func();
    }
    code[i] = NULL;
}

// program = "int" "*"? ident "(" ("int" ident ("," "int" ident)*)? ")" "{"
// stmt* "}"
Node *func() {
    expect("int");
    cur_func++;

    Type *type = get_var_type();

    Token *tok = consume_ident();

    if (tok == NULL) {
        error("トップレベルに関数または変数が定義されていません");
    }

    if (consume("(")) {
        return function_node(tok);
    }

    // 関数でない場合はグローバル変数定義
    Node *gvar = define_gvar(tok, type);
    expect(";");
    return gvar;
}

// グローバル変数定義
Node *define_gvar(Token *tok, Type *type) {
    Node *node = new_node(ND_GVAR, NULL, NULL);
    GVar *gvar = find_gvar(tok);
    if (gvar != NULL) {
        char name[100] = {0};
        strncpy(name, tok->str, tok->len);
        error("'%s'は既に定義されている変数です", name);
        return NULL;
    }

    return dispatch_define_gvar(tok, node, type);
}

// 関数定義
Node *function_node(Token *tok) {
    Node *node = new_node(ND_FUNC_DEF, NULL, NULL);
    // 引数
    if (consume("int")) {
        Token *argTok = consume_ident();
        Node *argHead = calloc(1, sizeof(Node));
        argHead->next = NULL;
        Node *argCur = argHead;

        while (argTok != NULL) {
            Node *argNext = calloc(1, sizeof(Node));
            argNext->str = argTok->str;
            argNext->len = argTok->len;
            argCur->argNext = argNext;
            argCur = argCur->argNext;

            Type *type = get_var_type();
            define_variable(argTok, argNext, type);

            consume(",");
            consume("int");
            argTok = consume_ident();
        }

        node->argNext = argHead->argNext;
    }

    expect(")");

    expect("{");
    // 関数の中身
    Node *head = calloc(1, sizeof(Node));
    head->next = NULL;
    Node *cur = head;

    while (!consume("}")) {
        cur->next = stmt();
        cur = cur->next;
    }

    node->str = tok->str;
    node->len = tok->len;
    node->next = head->next;
    return node;
}

// stmt = expr ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "return" expr ";"
//      | "int" "*"* ident ";"
Node *stmt() {
    Node *node;

    if (consume("return")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(";");
    } else if (consume("{")) {
        Node *head = calloc(1, sizeof(Node));
        head->next = NULL;
        Node *cur = head;

        while (!consume("}")) {
            cur->next = stmt();
            cur = cur->next;
        }

        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        node->next = head->next;
    } else if (consume("if")) {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (consume("else")) {
            node->els = stmt();
            node->kind = ND_IF_ELSE;
        }
    } else if (consume("while")) {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->cond = expr();
        expect(")");
        node->then = stmt();
    } else if (consume("for")) {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        if (!consume(";")) {
            node->ini = expr();
            expect(";");
        }

        if (!consume(";")) {
            node->cond = expr();
            expect(";");
        }

        if (!consume(")")) {
            node->inc = expr();
            expect(")");
        }

        node->then = stmt();
    } else if (consume("int")) {
        Type *type = get_var_type();
        Token *tok = consume_ident();
        node = calloc(1, sizeof(Node));
        define_variable(tok, node, type);
        expect(";");
        return node;
    } else {
        node = expr();
        expect(";");
    }

    return node;
}

// expr = assign
Node *expr() { return assign(); }

// assign     = equality ("=" assign)?
Node *assign() {
    Node *node = equality();

    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }

    return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("==")) {
            node = new_node(ND_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if (consume(">")) {
            node = new_node(ND_LT, add(), node);
        } else if (consume("<=")) {
            node = new_node(ND_LTE, node, add());
        } else if (consume(">=")) {
            node = new_node(ND_LTE, add(), node);
        } else {
            return node;
        }
    }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+")) {
            if (node->type != NULL && node->type->ty != INT) {
                int n = node->type->ptr_to->ty == INT ? INT_SIZE : PTR_SIZE;
                // *p + 3 => 3 * 4 = 12byteだけ進める
                Node *newNode = new_node(ND_MUL, mul(), new_node_num(n));
                node = new_node(ND_ADD, node, newNode);
            } else {
                node = new_node(ND_ADD, node, mul());
            }
        } else if (consume("-")) {
            if (node->type != NULL && node->type->ty != INT) {
                int n = node->type->ptr_to->ty == INT ? INT_SIZE : PTR_SIZE;
                Node *newNode = new_node(ND_MUL, mul(), new_node_num(n));
                node = new_node(ND_SUB, node, newNode);
            } else {
                node = new_node(ND_SUB, node, mul());
            }

        } else {
            return node;
        }
    }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

// 再起的にみていって、sizeofで必要となるタイプを取得する
Type *get_type(Node *node) {
    if (node == NULL) {
        return NULL;
    }

    if (node->type) {
        return node->type;
    }

    Type *t = get_type(node->lhs);
    if (t == NULL) {
        t = get_type(node->rhs);
    }

    if (t != NULL && node->kind == ND_DEREF) {
        t = t->ptr_to;
        if (t == NULL) {
            error("invalid dereference");
        }
    }

    return t;
}

// unary = "sizeof" unary |"+"? primary | "-"? primary | "*"? unary | "&"? unary
Node *unary() {
    if (consume("sizeof")) {
        Node *node = unary();
        Type *t = get_type(node);
        if (t != NULL && t->ty == PTR) {
            return new_node_num(PTR_SIZE);
        }

        return new_node_num(INT_SIZE);
    }

    if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    } else if (consume("&")) {
        return new_node(ND_ADDR, unary(), NULL);
    } else if (consume("*")) {
        return new_node(ND_DEREF, unary(), NULL);
    }

    return primary();
}

// primary = num
//         | ident ("(" (expr ("," expr)*)? ")")?
//         | "(" expr ")"
Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok == NULL) {
        return new_node_num(expect_number());
    }

    Node *node = calloc(1, sizeof(Node));

    // 関数呼び出し
    if (consume("(")) {
        node->kind = ND_FUNC_CALL;
        node->str = tok->str;
        node->len = tok->len;
        node->argNext = NULL;

        if (consume(")")) {
            // 引数がない関数呼び出し
            return node;
        }

        // 引数あり
        Node *head = calloc(1, sizeof(Node));
        head->argNext = NULL;
        Node *cur = head;

        while (!consume(")")) {
            cur->argNext = expr();
            cur = cur->argNext;
            consume(",");
        }

        node->argNext = head->argNext;
        return node;
    }

    // いずれにも当てはまらない場合は変数の値を取得
    node = lvar(tok, node);

    return node;
}

Type *get_var_type() {
    Type *type = calloc(1, sizeof(Type));
    type->ty = INT;
    type->ptr_to = NULL;
    while (consume("*")) {
        Type *next = calloc(1, sizeof(Type));
        next->ty = PTR;
        next->ptr_to = type;
        type = next;
    }

    return type;
}

void dispatch_define_lvar(Token *tok, Node *node, Type *type);

void define_variable(Token *tok, Node *node, Type *type) {
    LVar *lvar = find_lvar(tok);
    if (lvar != NULL) {
        char name[100] = {0};
        strncpy(name, tok->str, tok->len);
        error("'%s'は既に定義されている変数です", name);
        return;
    }

    dispatch_define_lvar(tok, node, type);
}

Node *lvar(Token *tok, Node *node) {
    LVar *lvar = find_lvar(tok);
    if (lvar == NULL) {
        char name[100] = {0};
        strncpy(name, tok->str, tok->len);
        error("'%s'が定義されていません", name);
        return NULL;
    }

    node->kind = ND_LVAR;
    node->offset = lvar->offset;
    node->type = lvar->type;

    // a[3] => *(a+3)
    // a[expr()] = *(a+expr())
    // TODO: 多重配列に対応
    if (consume("[")) {
        Node *add = new_node(ND_ADD, node, expr());
        Node *n = calloc(1, sizeof(Node));
        n->kind = ND_DEREF;
        n->lhs = add;
        node = n;
        expect("]");
    }

    return node;
}

void dispatch_define_lvar(Token *tok, Node *node, Type *type) {
    int size = type->ty == PTR ? PTR_SIZE : INT_SIZE;
    // 配列の場合
    while (consume("[")) {
        Type *t = calloc(1, sizeof(Type));
        t->ty = ARRAY;
        t->ptr_to = type;
        t->array_size = expect_number();
        size *= t->array_size;
        type = t;

        expect("]");
    }

    // 8の倍数にする
    while (size % 8 != 0) {
        size += 4;
    }

    LVar *lvar = calloc(1, sizeof(LVar));

    lvar->next = locals[cur_func];
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->type = type;

    if (locals[cur_func] == NULL) {
        lvar->offset = size;
    } else {
        lvar->offset = locals[cur_func]->offset + size;
    }

    node->kind = ND_LVAR;
    node->offset = lvar->offset;
    node->type = type;
    locals[cur_func] = lvar;
}

// グローバル変数の定義
Node *dispatch_define_gvar(Token *tok, Node *node, Type *type) {
    int size = type->ty == PTR ? PTR_SIZE : INT_SIZE;
    // 配列の場合
    while (consume("[")) {
        Type *t = calloc(1, sizeof(Type));
        t->ty = ARRAY;
        t->ptr_to = type;
        t->array_size = expect_number();
        size *= t->array_size;
        type = t;
        expect("]");
    }

    // 8の倍数にする
    while (size % 8 != 0) {
        size += 4;
    }

    GVar *gvar = calloc(1, sizeof(GVar));

    gvar->next = globals;
    gvar->name = tok->str;
    gvar->len = tok->len;
    gvar->type = type;
    globals = gvar;

    node->type = type;
    node->offset = size;
    node->str = tok->str;
    node->len = tok->len;

    return node;
}