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
void register_lvar(Token *tok, Node *node);

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

// program = ident "(" (ident ("," ident)*)? ")" "{" stmt* "}"
Node *func() {
    cur_func++;
    Node *node = new_node(ND_FUNC_DEF, NULL, NULL);
    Token *tok = consume_ident();

    if (tok == NULL) {
        error("トップレベルに関数が定義されていません");
    }

    expect("(");
    // 引数
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

        register_lvar(argTok, argNext);

        consume(",");
        argTok = consume_ident();
    }

    node->argNext = argHead->argNext;

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
//      | return expr ";"
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
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
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

// unary = ("+" | "-")? primary
Node *unary() {
    if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
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
        node->next = NULL;

        if (consume(")")) {
            // 引数がない関数呼び出し
            return node;
        }

        // 引数あり
        Node *head = calloc(1, sizeof(Node));
        head->next = NULL;
        Node *cur = head;

        while (!consume(")")) {
            cur->next = expr();
            cur = cur->next;
            consume(",");
        }

        node->next = head->next;
        return node;
    }

    // いずれにも当てはまらない場合は変数定義
    register_lvar(tok, node);

    return node;
}

void register_lvar(Token *tok, Node *node) {
    node->kind = ND_LVAR;
    LVar *lvar = find_lvar(tok);
    if (lvar) {
        node->offset = lvar->offset;
        return;
    }

    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals[cur_func];
    lvar->name = tok->str;
    lvar->len = tok->len;

    if (locals[cur_func] == NULL) {
        lvar->offset = VAR_SIZE;
    } else {
        lvar->offset = locals[cur_func]->offset + VAR_SIZE;
    }

    node->offset = lvar->offset;
    locals[cur_func] = lvar;
}