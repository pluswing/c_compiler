#include "9cc.h"

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    filename = argv[i];
    user_input = read_file(filename);
    Token *t = tokenize();
    if (!token) {
      token = t;
    } else {
      Token *tt = token;
      while (true) {
        if (tt->next->kind == TK_EOF) {
          tt->next = t;
          break;
        }
        tt = tt->next;
      }
    }
  }

  program();

  printf(".intel_syntax noprefix\n");
  printf(".bss\n");
  for (int i = 0; code[i]; i++) {
    if (code[i]->kind == ND_GVAR_DEF && !code[i]->var->init) {
      gen(code[i]);
    }
  }

  printf(".data\n");
  for (int i = 0; code[i]; i++) {
    if (code[i]->kind == ND_GVAR_DEF && code[i]->var->init) {
      gen(code[i]);
    }
  }
  for (StringToken *s = strings; s; s = s->next) {
    printf(".LC_%d:\n", s->index);
    printf("  .string \"%s\"\n", s->value);
  }

  printf(".text\n");
  cur_func = 0;
  for (int i = 0; code[i]; i++) {
    if (code[i]->kind == ND_FUNC_DEF) {
      cur_func++;
      gen(code[i]);
    }
  }

  return 0;
}
