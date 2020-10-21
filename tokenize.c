#include "9cc.h"

Token *token;
char *user_input;
char *filename;

char *read_file(char *path) {
  FILE *fp = fopen(path, "r");
  if (!fp) {
    error("cannot open %s: %s", path, strerror(errno));
  }
  if (fseek(fp, 0, SEEK_END) == -1) {
    error("%s: fseek %s", path, strerror(errno));
  }
  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1) {
    error("%s: fseek %s", path, strerror(errno));
  }

  char *buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  if (size == 0 || buf[size - 1] != '\n') {
    buf[size++] = '\n';
  }
  buf[size] = '\0';
  fclose(fp);
  return buf;
}

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

  char *line = loc;
  while(user_input < line && line[-1] != '\n') {
    line--;
  }

  char *end = loc;
  while (*end != '\n') {
    end++;
  }

  int line_num = 1;
  for (char *p = user_input; p < line; p++) {
    if (*p == '\n') {
      line_num++;
    }
  }

  int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);
  int pos = loc - line + indent;
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

bool peek(char *op) {
  return strlen(op) == token->len &&
      memcmp(token->str, op, token->len) == 0;
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
  {"char", TK_TYPE},
  {"sizeof", TK_SIZEOF},
  {"struct", TK_STRUCT},
  {"typedef", TK_TYPEDEF},
  {"enum", TK_ENUM},
  {"break", TK_BREAK},
  {"continue", TK_CONTINUE},
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

    // 行コメントスキップ
    if (startswith(p, "//")) {
      p += 2;
      while(*p != '\n') {
        p++;
      }
      continue;
    }

    // ブロックコメントをスキップ
    if (startswith(p, "/*")) {
      char *q = strstr(p + 2, "*/");
      if (!q) {
        error_at(p, "コメントが閉じられていません");
      }
      p = q + 2;
      continue;
    }

    if (startswith(p, "==") ||
        startswith(p, "!=") ||
        startswith(p, "<=") ||
        startswith(p, ">=") ||
        startswith(p, "+=") ||
        startswith(p, "-=") ||
        startswith(p, "/=") ||
        startswith(p, "*=") ||
        startswith(p, "++") ||
        startswith(p, "--") ||
        startswith(p, "->")) {

      cur = new_token(TK_RESRVED, cur, p, 2);
      p += 2;
      continue;
    }
    if (strchr("+-*/()<>=;{},&[].!~", *p)) {
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

    if ('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z') {
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

    if ('"' == *p) {
      p++;
      char *c = p;
      while('"' != *c) {
        c++;
      }
      int len = c - p;
      cur = new_token(TK_STRING, cur, p, len);
      p = c;
      p++;
      continue;
    }

    error_at(p, "expected a number");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

