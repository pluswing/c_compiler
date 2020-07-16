#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tokenize
typedef enum {
  TK_RESRVED, // 記号
  TK_IDENT, // 識別子
  TK_NUM, // 整数トークン
  TK_RETURN, // return
  TK_IF, // if
  TK_ELSE, // else
  TK_WHILE, // while
  TK_FOR, // for
  TK_EOF, // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

typedef struct LVar LVar;

struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

extern Token *token;
extern char *user_input;
extern LVar *locals[];
extern int cur_func;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_kind(TokenKind kind);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize();
LVar *find_lvar(Token *tok);

// codegen
typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_ASSIGN, // =
  ND_LVAR, // local variables
  ND_EQ,
  ND_NE,
  ND_LT, // <
  ND_LE, // <=
  ND_RETURN, // return
  ND_IF, // if
  ND_ELSE, // else
  ND_WHILE, // while
  ND_FOR, // for
  ND_FOR_LEFT,
  ND_FOR_RIGHT,
  ND_BLOCK, // { ... }
  ND_FUNC_CALL, // 関数呼び出し
  ND_FUNC_DEF, // 関数定義
  ND_ADDR, // &
  ND_DEREF, // *
  ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  Node **block; // only kind == ND_BLOCK
  char *funcname; // only kind == ND_FUNC*
  Node **args; // only kind == ND_FUNC_DEF
  int val;    // only kind == ND_NUM
  int offset; // only kind == ND_LVAR
};

extern Node *code[];

Node *new_node(NodeKind kind);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

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
Node* variable(Token *tok);

void gen_lval(Node *node);
void gen(Node *node);
