#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid args\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize();
  program();

  printf(".intel_syntax noprefix\n");
  printf(".bss\n");
  for (int i = 0; code[i]; i++) {
    if (code[i]->kind == ND_GVAR_DEF) {
      gen(code[i]);
    }
  }

  // TODO こういうのを出す
  // .LC0:
  // .string "abc"

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
