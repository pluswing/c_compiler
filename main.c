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
  printf(".globl main\n");
  printf(".bss\n");

  cur_func = 0;
  for (int i = 0; code[i]; i++) {
    cur_func++;
    if (cur_func == 3) {
      printf(".text\n");
    }
    gen(code[i]);
  }

  return 0;
}
