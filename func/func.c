#include <stdio.h>
#include <stdlib.h>

int foo() {
  printf("OK\n");
  return 1;
}

int bar(int x, int y) {
  printf("%d\n", x + y);
  return x + y;
}

int bar2(int x, int y, int z) {
  printf("%d + %d + %d = %d\n", x, y, z, x + y + z);
  return x + y + z;
}

void alloc4(int **p, int a, int b, int c, int d) {
  *p = malloc(sizeof(int) * 4);
  (*p)[0] = a;
  (*p)[1] = b;
  (*p)[2] = c;
  (*p)[3] = d;
}
