#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
  TK_TYPE, // int
  TK_SIZEOF, // sizeof
  TK_STRING, // 文字列
  TK_STRUCT, // 構造体
  TK_TYPEDEF, // typedef
  TK_ENUM, // enum
  TK_BREAK, // break
  TK_CONTINUE, // continue
  TK_SWITCH, // switch
  TK_CASE, // case
  TK_DEFAULT, // default
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


typedef struct Member Member;
typedef struct Type Type;

struct Member {
  Member *next;
  Type *ty;
  char *name;
  int offset;
};

typedef enum {
  INT,
  PTR,
  ARRAY,
  CHAR,
  STRUCT
} TypeKind;

struct Type {
  TypeKind ty;
  struct Type *ptr_to;
  size_t array_size;
  Member *members;
  int size;
  bool incomplete;
};

typedef struct LVar LVar;

struct Node;
struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
  Type *type;
  enum { LOCAL, GLOBAL } kind;
  struct Node *init;
};

typedef struct Define Define;
struct Define {
  Type *type;
  Token *ident;
};

typedef struct StringToken StringToken;
struct StringToken {
  int index;
  char *value;
  StringToken *next;
};

typedef struct Tag Tag;
struct Tag {
  Tag *next;
  char *name;
  Type *type;
};

typedef struct EnumVar EnumVar;
struct EnumVar {
  EnumVar *next;
  char *name;
  int value;
};

extern Token *token;
extern char *user_input;
extern char *filename;

extern LVar *locals[];
extern LVar *globals[];
extern int cur_func;
extern Tag *tags;
extern EnumVar *enum_vars;

char *read_file(char *path);
void error(char *fmt);
void error1(char *fmt, char *v1);
void error2(char *fmt, char *v1, char *v2);
void error_at(char *loc, char *fmt);
void error_at_s(char *loc, char *fmt, char *val);
bool consume(char *op);
bool peek(char *op);
Token *consume_kind(TokenKind kind);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize();
Token *read_char_literal(Token *cur, char *start);
char get_escape_char(char c);

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
  ND_GVAR_DEF, // グローバル変数の定義
  ND_GVAR, // グローバル変数の使用
  ND_STRING, // 文字列
  ND_MEMBER,
  ND_BREAK, // break
  ND_CONTINUE, // continue
  ND_NOT, // !
  ND_BITNOT, // ~
  ND_BITAND, // &
  ND_BITOR, // |
  ND_BITXOR, // ^
  ND_LOGOR, // ||
  ND_LOGAND, // &&
  ND_TERNARY, // ?:
  ND_TERNARY_R, // ?:
  ND_SWITCH, // switch
  ND_CASE, // case
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
  Type *type; // only kind == ND_LVAR
  char *varname; // only kind == ND_*VAR
  int size;      // only kind == ND_*VAR
  StringToken *string; // only kind == ND_STRING
  LVar *var;           // only kind == ND_GVAR_DEF
  Member *member; // only kind == ND_MEMBER

  // switch-case
  Node *case_next;
  Node *default_case;
  int case_label;
};

extern Node *code[];
extern StringToken *strings;

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
Node *local_variable_init(Node *node);
Node *define_variable();
Node *variable(Token *tok);
Type *get_type(Node *node);
Define *read_define();
Type *read_type();
LVar *find_variable(Token *tok);
Type *define_struct();
void read_type_suffix(Define *def);
int get_size(Type *type);
Member *find_member(Token *token, Type* type);
int align_to(int n, int align);
void push_tag(char *prefix, Token *token, Type *type, bool is_typedef);
Tag *find_tag(char *prefix, Token *token);
Node *struct_ref(Node *node);
bool define_typedef();
Type *define_enum();
Type *int_type();
Node *find_enum_var(Token *tok);
Node *ptr_calc(Node* node, Node *r);
Node *bitor();
Node *bitxor();
Node *bitand();
Node *logor();
Node *logand();
Node *conditional();

void gen_val(Node *node);
void gen(Node *node);
