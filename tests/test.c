int assert(int expected, int actual) {

  if (expected == actual) {
    printf(".");
    return 0;
  }
  printf("%d expected, but got %d\n", expected, actual);
  exit(1);
}

int main() {
  // テスト本体

  assert(42, 42);
  assert(21, 5+20-4);
  assert(41, 12 + 34 - 5 );
  assert(47, 5+6*7);
  assert(15, 5*(9-6));
  assert(4, (3+5)/2);
  assert(10, -10+20);
  assert(10, - -10);
  assert(10, - - +10);

  printf("OK\n");
  return 0;
}
