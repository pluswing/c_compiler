int g_a;
int g_b[10];
int g_a_init3 = 3;
int g_array[5] = {0, 1, 2, 3, 4};
char g_array2[5] = {5, 6, 7, 8, 12};
char *g_msg1 = "foo";
char g_msg2[4] = "bar";

int assert(int expected, int actual) {

  if (expected == actual) {
    printf(".");
    return 0;
  }
  printf("%d expected, but got %d\n", expected, actual);
  exit(1);
}

int fail() {
  printf("called fail!\n");
  exit(1);
}

int ok() {
  printf(".");
}

int test_calc() {
  assert(42, 42);
  assert(21, 5+20-4);
  assert(41, 12 + 34 - 5 );
  assert(47, 5+6*7);
  assert(15, 5*(9-6));
  assert(4, (3+5)/2);
  assert(10, -10+20);
  assert(10, - -10);
  assert(10, - - +10);
}

int test_compare() {
  assert(0, 0==1);
  assert(1, 42==42);
  assert(1, 0!=1);
  assert(0, 42!=42);

  assert(1, 0<1);
  assert(0, 1<1);
  assert(0, 2<1);
  assert(1, 0<=1);
  assert(1, 1<=1);
  assert(0, 2<=1);

  assert(1, 1>0);
  assert(0, 1>1);
  assert(0, 1>2);
  assert(1, 1>=0);
  assert(1, 1>=1);
  assert(0, 1>=2);
}

int test_variable() {
  int foo;
  int bar;
  foo = 3;
  bar = 5 * 6 - 8;
  assert(3, foo);
  assert(bar, 22);
  assert(14, foo + bar / 2);
}

int inner_test_return() {
  int a;
  int b;
  a = 3;
  b = 5 * 6 - 8;
  return a + b / 2;
}

int inner_multi_return() {
  return 5;
  return 8;
}

int test_return() {
  assert(14, inner_test_return());
  assert(5, inner_multi_return());
}

int test_if() {
  if (3 != 3) {
    fail();
  }

  int a;
  a = 3;
  if (a == 3) {
    ok();
    return 0;
  }
  fail();
}

int test_while() {
  int i;
  i = 0;
  while (i <= 10) i = i + 1;
  assert(11, i);
}

int test_for() {
  int a;
  int i;
  a = 0;
  for (i = 0; i < 10; i = i + 1) a = a + 2;
  assert(10, i);
  assert(20, a);

  int b;
  b = 0;
  for (;b < 10;) b = b + 1;
  assert(10, b);
}

int inner_multi_c() {
   int b;
  b = 0;
  for(;;b = b + 1) if (b == 5) return b;
}
int test_multi_control_syntax() {
  int a;
  a = 3;
  if (a == 1) fail();
  if (a == 2) fail();
  if (a == 3) ok();
  assert(3, a);

  assert(5, inner_multi_c());
}

int inner_block() {
  int a;
  a = 0;
  for(;;) {
    a = a + 1;
    if (a == 5) return 10;
  }
  return 2;
}

int test_block() {
  assert(10, inner_block());
}


int foo() {
  return 1;
}

int bar(int x, int y) {
  return x + y;
}

int bar2(int x, int y, int z) {
  return x + y + z;
}

int test_func() {
  assert(1, foo());
  assert(7, bar(3, 4));
  assert(12, bar2(3, 4, 5));
}

int inner_test_pointer01() {
  int x;
  int *y;
  x = 3;
  y = &x;
  assert(3, *y);
}
int inner_test_pointer02() {
  int x;
  int y;
  int *z;
  x = 3;
  y = 5;
  z = &y + 4;
  assert(3, *z);
}
int test_pointer() {
  inner_test_pointer01();
  inner_test_pointer02();
}

int test_func_def_func1(int a, int b) { return a + b; }
int test_func_def_func2(int a, int b, int c) { return a + c; }
int test_func_def() {
  assert(3, test_func_def_func1(1, 2));
  assert(4, test_func_def_func2(1, 2, 3));
}

int sum(int n) {
  if (n < 0) return 0;
  return n + sum(n - 1);
}

int test_func_def_recursive() {
  int a;
  a = 10;
  assert(55, sum(a));
}

int test_pointer_calc() {
  int *p;
  alloc4(&p, 1, 2, 4, 8);

  int *q;
  q = p + 2;
  assert(4, *q);

  q = p + 3;
  assert(8, *q);

  q = q - 2;
  assert(2, *q);
}

int test_sizeof() {
  int x;
  assert(4, sizeof(x));

  int *y;
  assert(8, sizeof(y));

  assert(8, sizeof(y + 3));

  assert(4, sizeof(*y));

  assert(4, sizeof(1));
}

int test_array() {
  int a[2];
  *a = 1;
  *(a + 1) = 2;
  int *p;
  p = a;
  assert(3, *p + *(p + 1));
}

int test_array_access() {
  int a[2];
  a[0] = 1;
  a[1] = 2;
  int *p;
  p = a;
  assert(3, p[0] + p[1]);
}

int test_global_variable() {
  g_a = 10;
  assert(10, g_a);
}

int test_char() {
  char x[3];
  x[0] = -1;
  x[1] = 2;
  int y;
  y = 4;
  assert(3, x[0] + y);
  assert(1, sizeof(x[0]));
}

int test_string() {
  char *a;
  a = "abcd";
  assert(97, a[0]);
}

int test_gloval_variable_init() {
  assert(3, g_a_init3);

  assert(0, g_array[0]);
  assert(4, g_array[4]);

  assert(5, g_array2[0]);
  assert(12, g_array2[4]);

  assert(102, g_msg1[0]);
  assert(111, g_msg1[2]);

  assert(98, g_msg2[0]);
  assert(114, g_msg2[2]);
}

int test_local_variable_init() {
  int a = 10;
  assert(10, a);

  int b[3] = {1, 2, 7};
  assert(1, b[0]);
  assert(2, b[1]);
  assert(7, b[2]);
}

int main() {

  test_calc();
  test_compare();
  test_variable();
  test_return();
  test_if();
  test_while();
  test_for();
  test_multi_control_syntax();
  test_block();
  test_func();
  test_pointer();
  test_func_def();
  test_func_def_recursive();
  test_pointer_calc();
  test_sizeof();
  test_array();
  test_array_access();
  test_global_variable();
  test_char();
  test_string();
  test_gloval_variable_init();
  test_local_variable_init();

  printf("OK\n");
  return 0;
}
