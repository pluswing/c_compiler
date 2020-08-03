#include "9cc.h"

Token *token;
char *user_input;
LVar *locals[100];
int cur_func = 0;

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool consume(char *op) {
  if (token->kind != TK_RESRVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {

    return false;
  }
  token = token->next;
  return true;
}

Token* consume_kind(TokenKind kind) {
  if (token->kind != kind) {
    return NULL;
  }
  Token* tok = token;
  token = token->next;
  return tok;
}

void expect(char *op) {
  if (token->kind != TK_RESRVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {

    error_at(token->str, "\"%s\"ではありません", op);
  }
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "数字ではありません");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

typedef struct ReservedWord ReservedWord;
struct ReservedWord {
  char *word;
  TokenKind kind;
};

ReservedWord reservedWords[] = {
  {"return", TK_RETURN},
  {"if", TK_IF},
  {"else", TK_ELSE},
  {"while", TK_WHILE},
  {"for", TK_FOR},
  {"int", TK_TYPE},
  {"sizeof", TK_SIZEOF},
  {"", TK_EOF},
};

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

    if (startswith(p, "==") ||
        startswith(p, "!=") ||
        startswith(p, "<=") ||
        startswith(p, ">=")) {

      cur = new_token(TK_RESRVED, cur, p, 2);
      p += 2;
      continue;
    }
    if (strchr("+-*/()<>=;{},&[]", *p)) {
      cur = new_token(TK_RESRVED, cur, p++, 1);
      continue;
    }

    bool found = false;
    for (int i = 0; reservedWords[i].kind != TK_EOF; i++) {
      char *w = reservedWords[i].word;
      int len = strlen(w);
      TokenKind kind = reservedWords[i].kind;
      if (startswith(p, w) && !is_alnum(p[len])) {
        cur = new_token(kind, cur, p, len);
        p += len;
        found = true;
        break;
      }
    }
    if (found) {
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      char *c = p;
      while(is_alnum(*c)) {
        c++;
      }
      int len = c - p;
      cur = new_token(TK_IDENT, cur, p, len);
      p = c;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 1);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "expected a number");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals[cur_func]; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {

      return var;
    }
  }
  return NULL;
}