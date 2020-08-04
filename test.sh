#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cd func
  cc -c func.c
  cd ..
  cc -o tmp tmp.s func/func.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

# global variables
assert 1 "
int a;
int b[10];
int main() {
  return 1;
}
"

# array access
# FIXME 2重配列はNGだった。
assert 3 "int main() {
  int a[2];
  a[0] = 1;
  a[1] = 2;
  int *p;
  p = a;
  return p[0] + p[1];
}"

# array pointer
assert 3 "int main() {
  int a[2];
  *a = 1;
  *(a + 1) = 2;
  int *p;
  p = a;
  return *p + *(p + 1);
}"

# array
assert 0 "int main() {
  int a[10];
  return 0;
}"

# sizeof
assert 4 "int main() {
  int x;
  return sizeof(x);
}"
assert 8 "int main() {
  int *x;
  return sizeof(x);
}"
assert 8 "int main() {
  int *x;
  return sizeof(x + 3);
}"
assert 4 "int main() {
  int *x;
  return sizeof(*x);
}"
assert 4 "int main() {
  return sizeof(1);
}"

#pointer +/-
assert 4 "int main() {
  int *p;
  alloc4(&p, 1, 2, 4, 8);
  int *q;
  q = p + 2;
  return *q;
}"
assert 8 "int main() {
  int *p;
  alloc4(&p, 1, 2, 4, 8);
  int *q;
  q = p + 3;
  return *q;
}"
assert 2 "int main() {
  int *p;
  alloc4(&p, 1, 2, 4, 8);
  int *q;
  q = p + 3;
  q = q - 2;
  return *q;
}"

#pointer
assert 3 "int main() {
  int x;
  int *y;
  y = &x;
  *y = 3;
  return x;
}
"

# int
assert 2 "int main() {
  int x;
  x = 2;
  return 2;
}"
assert 6 "int main() {
  int x;
  x = 2;
  return func(x, 4);
}
int func(int x, int y) {
  return x + y;
}"

# compare calc tests
assert 2 "int main () { return 2;}"
assert 2 "int main () return 2;"
assert 3 "
int main() return func(1, 2);
int func(int a, int b) { return a + b; }
"
assert 4 "
int main() return func(1, 2, 3);
int func(int a, int b, int c) { return a + c; }
"

assert 55 "int main () {
  int a;
  a = 10;
  return sum(a);
}
int sum(int n) {
  if (n < 0) return 0;
  return n + sum(n - 1);
}
"

assert 42 "int main() return 42;"
assert 21 "int main() return 5+20-4;"
assert 41 "int main() return  12 + 34 - 5 ;"
assert 47 "int main() return 5+6*7;"
assert 15 "int main() return 5*(9-6);"
assert 4 "int main() return (3+5)/2;"
assert 10 "int main() return -10+20;"
assert 10 "int main() return - -10;"
assert 10 "int main() return - - +10;"

# compare tests(1)
assert 0 "int main() return 0==1;"
assert 1 "int main() return 42==42;"
assert 1 "int main() return 0!=1;"
assert 0 "int main() return 42!=42;"

# compare tests(2)
assert 1 "int main() return 0<1;"
assert 0 "int main() return 1<1;"
assert 0 "int main() return 2<1;"
assert 1 "int main() return 0<=1;"
assert 1 "int main() return 1<=1;"
assert 0 "int main() return 2<=1;"

# compare tests(3)
assert 1 "int main() return 1>0;"
assert 0 "int main() return 1>1;"
assert 0 "int main() return 1>2;"
assert 1 "int main() return 1>=0;"
assert 1 "int main() return 1>=1;"
assert 0 "int main() return 1>=2;"

# variable tests
assert 14 "int main() {
int a;
int b;
a = 3;
b = 5 * 6 - 8;
a + b / 2;
}"
assert 6 "int main() {
int foo;
int bar;
foo = 1;
bar = 2 + 3;
foo + bar;
}"

# return tests
assert 14 "int main() {
int a;
int b;
a = 3;
b = 5 * 6 - 8;
return a + b / 2;
}"
assert 5 "int main() {return 5;
return 8;
}"

# IF tests
assert 3 "int main() {
int a;
a = 3;
if (a == 3) return a;
return 5;
}
"
assert 5 "int main() {if (3 != 3) return 1;
return 5;
}
"
assert 5 "int main() {if (3 != 3) return 1;
else return 5;
return 2;
}
"

# while
assert 11 "int main() {
int i;
i = 0;
while (i <= 10) i = i + 1;
return i;
}"

# for
assert 30 "int main() {
int a;
int i;
a = 0;
for (i = 0; i < 10; i = i + 1) a = a + 2;
return i + a;
}"
assert 10 "int main() {
int a;
a = 0;
for (;a < 10;) a = a + 1;
return a;
}"

# multi contorl syntax
assert 6 "int main() {
int a;
a = 3;
if (a == 1) return 4;
if (a == 2) return 5;
if (a == 3) return 6;
}"
assert 10 "int main() {
int a;
a = 0;
for(;;a = a + 1) if (a == 5) return 10;
return 2;
}"

# block
assert 10 "int main() {
int a;
a = 0;
for(;;) {
  a = a + 1;
  if (a == 5) return 10;
}
return 2;
}"

# func
assert 1 "int main() return foo();"
assert 7 "int main() return bar(3, 4);"
assert 12 "int main() return bar2(3, 4, 5);"

# &, *
assert 3 "int main() {
  int x;
  int y;
  x = 3;
  y = &x;
  return *y;
}"
assert 3 "int main() {
  int x;
  int y;
  int z;
  x = 3;
  y = 5;
  z = &y + 8;
  return *z;
}"

echo OK
