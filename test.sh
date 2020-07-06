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

# compare calc tests
assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 10 "-10+20;"
assert 10 "- -10;"
assert 10 "- - +10;"

# compare tests(1)
assert 0 "0==1;"
assert 1 "42==42;"
assert 1 "0!=1;"
assert 0 "42!=42;"

# compare tests(2)
assert 1 "0<1;"
assert 0 "1<1;"
assert 0 "2<1;"
assert 1 "0<=1;"
assert 1 "1<=1;"
assert 0 "2<=1;"

# compare tests(3)
assert 1 "1>0;"
assert 0 "1>1;"
assert 0 "1>2;"
assert 1 "1>=0;"
assert 1 "1>=1;"
assert 0 "1>=2;"

# variable tests
assert 14 "a = 3;
b = 5 * 6 - 8;
a + b / 2;"
assert 6 "foo = 1;
bar = 2 + 3;
foo + bar;"

# return tests
assert 14 "a = 3;
b = 5 * 6 - 8;
return a + b / 2;"
assert 5 "return 5;
return 8;"

# IF tests
assert 3 "a = 3;
if (a == 3) return a;
return 5;
"
assert 5 "if (3 != 3) return 1;
return 5;
"
assert 5 "if (3 != 3) return 1;
else return 5;
return 2;
"

# while
assert 11 "i = 0;
while (i <= 10) i = i + 1;
return i;"

# for
assert 30 "a = 0;
for (i = 0; i < 10; i = i + 1) a = a + 2;
return i + a;"
assert 10 "a = 0;
for (;a < 10;) a = a + 1;
return a;"

# multi contorl syntax
assert 6 "a = 3;
if (a == 1) return 4;
if (a == 2) return 5;
if (a == 3) return 6;"
assert 10 "a = 0;
for(;;a = a + 1) if (a == 5) return 10;
return 2;"

# block
assert 10 "a = 0;
for(;;) {
  a = a + 1;
  if (a == 5) return 10;
}
return 2;"

# func
assert 0 "foo();"
assert 0 "bar(3, 4);"
assert 0 "bar2(3, 4, 5);"

echo OK
