#include "9cc.h"

Token *token;
char *user_input;
char *filename;

char *token2string(Token *token) {
  char *str = calloc(token->len + 1, sizeof(char));
  memcpy(str, token->str, token->len);
  return str;
}

char *read_file(char *path) {
  FILE *fp = fopen(path, "r");
  if (!fp) {
    error2("cannot open %s: %s", path, strerror(errno));
  }
  if (fseek(fp, 0, SEEK_END) == -1) {
    error2("%s: fseek %s", path, strerror(errno));
  }
  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1) {
    error2("%s: fseek %s", path, strerror(errno));
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

void error(char *fmt) {
  fprintf(stderr, fmt, "");
  fprintf(stderr, "\n");
  exit(1);
}

void error1(char *fmt, char *v1) {
  fprintf(stderr, fmt, v1);
  fprintf(stderr, "\n");
  exit(1);
}

void error2(char *fmt, char *v1, char *v2) {
  fprintf(stderr, fmt, v1, v2);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt) {
  error_at_s(loc, fmt, NULL);
}

void error_at_s(char *loc, char *fmt, char *val) {
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
  fprintf(stderr, fmt, val);
  fprintf(stderr, "\n");
  exit(1);
}

bool consume(char *op) {
  if (token->kind != TK_RESRVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {

    return false;
  }
  fprintf(stderr, "%s ", token2string(token));
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
  fprintf(stderr, "%s ", token2string(token));
  token = token->next;
  return tok;
}

void expect(char *op) {
  if (token->kind != TK_RESRVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {

    error_at_s(token->str, "'%s'ではありません", op);
  }
  fprintf(stderr, "%s ", token2string(token));
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "数字ではありません");
  }
  int val = token->val;
  fprintf(stderr, "%s ", token2string(token));
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
  {"void", TK_TYPE}, // int alias
  {"size_t", TK_TYPE}, // int alias
  {"bool", TK_TYPE}, // int alias
  {"FILE", TK_TYPE},
  {"sizeof", TK_SIZEOF},
  {"struct", TK_STRUCT},
  {"typedef", TK_TYPEDEF},
  {"enum", TK_ENUM},
  {"break", TK_BREAK},
  {"continue", TK_CONTINUE},
  {"switch", TK_SWITCH},
  {"case", TK_CASE},
  {"default", TK_DEFAULT},
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

    // #includeは読み飛ばす
    if (startswith(p, "#include")) {
      while(*p != '\n') {
        p++;
      }
      continue;
    }
    // externも読み飛ばす。
    if (startswith(p, "extern")) {
      while(*p != '\n') {
        p++;
      }
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
        startswith(p, "||") ||
        startswith(p, "&&") ||
        startswith(p, "->")) {

      cur = new_token(TK_RESRVED, cur, p, 2);
      p += 2;
      continue;
    }

    // char literal
    if (*p == '\'') {
      cur = read_char_literal(cur, p);
      p += cur->len;
      continue;
    }

    if (strchr("+-*/()<>=;{},&[].!~|^?:", *p)) {
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
      while (true) {
        if (startswith(c, "\\\"")) {
          c += 2;
          continue;
        }
        if (*c == '"') {
          break;
        }
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

Token *read_char_literal(Token *cur, char *start) {
  char *p = start + 1;
  if (*p == '\0') {
    error_at(start, "unclosed char literal");
  }

  char c;
  if (*p == '\\') {
    p++;
    c = get_escape_char(*p++);
  } else {
    c = *p;
    p++;
  }

  if (*p != '\'') {
    error_at(start, "char literal too long");
  }
  p++;

  Token *tok = new_token(TK_NUM, cur, start, p - start);
  tok->val = c;
  return tok;
}

char get_escape_char(char c) {
  switch (c) {
  case 'a': return '\a';
  case 'b': return '\b';
  case 't': return '\t';
  case 'n': return '\n';
  case 'v': return '\v';
  case 'f': return '\f';
  case 'r': return '\r';
  case 'e': return 27;
  case '0': return 0;
  default: return c;
  }
}
