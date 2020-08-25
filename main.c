#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid args\n");
    return 1;
  }

  filename = argv[1];
  user_input = read_file(filename);
  token = tokenize();
  program();

  printf(".intel_syntax noprefix\n");
  printf(".bss\n");
  for (int i = 0; code[i]; i++) {
    if (code[i]->kind == ND_GVAR_DEF) {
      gen(code[i]);
    }
  }

  printf(".data\n");
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
