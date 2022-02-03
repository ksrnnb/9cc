#include <stdlib.h>
#include <string.h>

#include "9cc.h"

#define IDENT_SIZE 8

void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// 解析したstatementを格納する
Node *code[100];

// 現在みているトークン
Token *token;

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
        code[i++] = stmt();
    }
    code[i] = NULL;
}

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

Node *expr() { return assign(); }

Node *assign() {
    Node *node = equality();

    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }

    return node;
}

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

Node *unary() {
    if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    }

    return primary();
}

// primary = num
//         | ident ("(" ")")?
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

    if (consume("(")) {
        expect(")");
        node->kind = ND_FUNC_CALL;
        node->str = tok->str;
        node->len = tok->len;

        return node;
    }

    node->kind = ND_LVAR;
    LVar *lvar = find_lvar(tok);
    if (lvar) {
        node->offset = lvar->offset;
        return node;
    }

    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->offset = locals->offset + IDENT_SIZE;
    node->offset = lvar->offset;
    locals = lvar;

    return node;
}