int g_a;
int g_b[10];
int g_a_init3 = 3;
int g_array[5] = {0, 1, 2, 3, 4};
char g_array2[5] = {5, 6, 7, 8, 12};
char *g_msg1 = "foo";
char g_msg2[4] = "bar";
// TODO これたぶん未対応
//char *a[] = {"abc", "def"};

struct Hoge {
  int a;
  char b;
  int c;
};

typedef int Int;
typedef char* String;
String strtest = "cccc";
typedef struct Hoge StructHoge;

typedef struct Nest Nest;
struct Nest {
  struct Nest *next;
  int a;
};

enum HogeEnum {
  AAA = 10,
  BBB,
  CCC
};

enum LastComma {
  AAA,
  BBB, // 最後にカンマはあっても良い。
};

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

  assert(4, sizeof(int));
  assert(1, sizeof(char));

  struct SizeOfTest {
    int a;
    char b;
  } a;

  // struct
  // TODO 要確認
  assert(8, sizeof(a));
  assert(8, sizeof(struct SizeOfTest));

  // typedef
  assert(4, sizeof(Int));
  assert(8, sizeof(String));
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

  int c[] = { 1, 2 };
  int d[5] = { 5 };
  int e[3] = { 3, 4, 5 };

  assert(1, c[0]);
  assert(2, c[1]);

  assert(5, d[0]);
  assert(0, d[1]);

  assert(3, e[0]);
  assert(5, e[2]);

  char abc[10] = "abc";
  char def[] = "def";

  assert(97, abc[0]);
  assert(99, abc[2]);

  assert(100, def[0]);
  assert(102, def[2]);
}

int test_struct() {
  struct {int a; int b;} abc;
  abc.a = 10;
  abc.b = 20;
  assert(10, abc.a);
  assert(20, abc.b);

  struct StTest {int a; char b; int c;} ccc;
  int size = &ccc.c - &ccc.a;
  assert(8, size);

  struct StTest ccc2;
  int size2 = &ccc2.c - &ccc2.a;
  assert(8, size2);

  ccc2.a = 7;
  struct StTest *ptr = &ccc2;
  assert(7, ptr->a);
}

int test_typedef() {
  Int b;
  b = 10;
  assert(10, b);

  // struct Hoge hoge;
  StructHoge hoge;
  hoge.a = 10;
  hoge.b = 20;
  assert(10, hoge.a);
  assert(20, hoge.b);
}

int test_enum() {
  enum HogeEnum2 {
    AAA = 10,
    BBB,
    CCC
  } aa;
  enum HogeEnum2 hoge;
  hoge = AAA;
  assert(10, hoge);
  assert(11, BBB);
  assert(12, CCC);
}

int test_break() {
  int i = 0;
  for(;;) {
    i = i + 1;
    if (i == 3) {
      break;
    }
  }
  assert(3, i);

  i = 0;
  for (;;) {
    int j = 0;
    i = i + 1;
    while(i) {
      j = j + 1;
      if (j == 2) {
        break;
      }
    }
    assert(2, j);
    if (i == 3) {
      break;
    }
  }
  assert(3, i);
}

int test_continue() {
  int i = 0;
  int j = 0;
  while(i < 10) {
    i = i + 1;
    if (i > 5) {
      continue;
    }
    j = j + 1;
  }
  assert(10, i);
  assert(5, j);

  i = 0;
  j = 0;
  for(;i < 10;i = i + 1) {
    if (i > 5) {
      continue;
    }
    j = j + 1;
  }
  assert(10, i);
  assert(5, j);
}

int test_addeq() {
  int i = 2;
  i += 5;
  assert(7, i);

  i -= 3;
  assert(4, i);

  i *= 2;
  assert(8, i);

  i /= 2;
  assert(4, i);
}

int test_addeq_ptr() {
  int a[10] = {1,2,3,4,5};
  int *ptr = &a;
  assert(1, *ptr);
  ptr += 4;
  assert(5, *ptr);
  ptr -= 3;
  assert(2, *ptr);
}

int test_pp() {
  int a = 10;
  assert(10, a++);
  assert(11, a);

  assert(12, ++a);
  assert(12, a);

  assert(12, a--);
  assert(11, a);

  assert(10, --a);
  assert(10, a);
}

int test_bit() {
  // not
  assert(0, !1);
  assert(0, !2);
  assert(1, !0);

  // bit not
  assert(-1, ~0);
  assert(0, ~-1);

  // bit and
  assert(0, 0&1);
  assert(1, 3&1);
  assert(3, 7&3);
  assert(10, -1&10);

  // bit or
  assert(0, 0|0);
  assert(1, 0|1);
  assert(1, 1|1);

  // bit xor
  assert(0, 0^0);
  assert(1, 1^0);
  assert(0, 1^1);

}

int test_and_or() {
  assert(1, 0||1);
  assert(1, 0||(2-2)||5);
  assert(0, 0||0);
  assert(0, 0||(2-2));

  assert(0, 0&&1);
  assert(0, (2-2)&&5);
  assert(1, 1&&5);
}

int test_ternary() {
  assert(2, 0?1:2);
  assert(1, 1?1:2);
}

int test_switch() {
  int i = 0;
  switch(0) {
  case 0:
    i = 5;
    break;
  case 1:
    i = 6;
    break;
  case 2:
    i = 7;
    break;
  }
  assert(5, i);

  switch(5) {
  case 1:
  case 2:
    break;
  default:
    i = 10;
  }
  assert(10, i);
}

void test_void() {
  // コンパイルが通ればOK。
  assert(1, 1);
}

void test_nest_types() {
  Nest a;
  Nest *b;
  a.a = 10;
  b = &a;
  assert(10, b->a);

  Nest c;
  c.a = 20;
  a.next = &c;
  assert(20, b->next->a);

  Nest d;
  d.a = 30;
  c.next = &d;
  assert(30, b->next->next->a);
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
  test_struct();
  test_typedef();
  test_enum();
  test_break();
  test_continue();
  test_addeq();
  test_addeq_ptr();
  test_pp();
  test_bit();
  test_and_or();
  test_ternary();
  test_switch();
  test_void();
  test_nest_types();

  printf("OK\n");
  return 0;
}
