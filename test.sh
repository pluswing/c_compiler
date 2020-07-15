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
assert 2 "main () { return 2;}"
assert 2 "main () return 2;"
assert 3 "
main() return func(1, 2);
func(a, b) { return a + b; }
"
assert 4 "
main() return func(1, 2, 3);
func(a, b, c) { return a + c; }
"

assert 55 "main () {
  a = 10;
  return sum(a);
}
sum(n) {
  if (n < 0) return 0;
  return n + sum(n - 1);
}
"

assert 42 "main() return 42;"
assert 21 "main() return 5+20-4;"
assert 41 "main() return  12 + 34 - 5 ;"
assert 47 "main() return 5+6*7;"
assert 15 "main() return 5*(9-6);"
assert 4 "main() return (3+5)/2;"
assert 10 "main() return -10+20;"
assert 10 "main() return - -10;"
assert 10 "main() return - - +10;"

# compare tests(1)
assert 0 "main() return 0==1;"
assert 1 "main() return 42==42;"
assert 1 "main() return 0!=1;"
assert 0 "main() return 42!=42;"

# compare tests(2)
assert 1 "main() return 0<1;"
assert 0 "main() return 1<1;"
assert 0 "main() return 2<1;"
assert 1 "main() return 0<=1;"
assert 1 "main() return 1<=1;"
assert 0 "main() return 2<=1;"

# compare tests(3)
assert 1 "main() return 1>0;"
assert 0 "main() return 1>1;"
assert 0 "main() return 1>2;"
assert 1 "main() return 1>=0;"
assert 1 "main() return 1>=1;"
assert 0 "main() return 1>=2;"

# variable tests
assert 14 "main() {a = 3;
b = 5 * 6 - 8;
a + b / 2;
}"
assert 6 "main() {foo = 1;
bar = 2 + 3;
foo + bar;
}"

# return tests
assert 14 "main() {a = 3;
b = 5 * 6 - 8;
return a + b / 2;
}"
assert 5 "main() {return 5;
return 8;
}"

# IF tests
assert 3 "main() {a = 3;
if (a == 3) return a;
return 5;
}
"
assert 5 "main() {if (3 != 3) return 1;
return 5;
}
"
assert 5 "main() {if (3 != 3) return 1;
else return 5;
return 2;
}
"

# while
assert 11 "main() { i = 0;
while (i <= 10) i = i + 1;
return i;
}"

# for
assert 30 "main() {a = 0;
for (i = 0; i < 10; i = i + 1) a = a + 2;
return i + a;
}"
assert 10 "main() {a = 0;
for (;a < 10;) a = a + 1;
return a;
}"

# multi contorl syntax
assert 6 "main() {a = 3;
if (a == 1) return 4;
if (a == 2) return 5;
if (a == 3) return 6;
}"
assert 10 "main() {a = 0;
for(;;a = a + 1) if (a == 5) return 10;
return 2;
}"

# block
assert 10 "main() {a = 0;
for(;;) {
  a = a + 1;
  if (a == 5) return 10;
}
return 2;
}"

# func
assert 1 "main() return foo();"
assert 7 "main() return bar(3, 4);"
assert 12 "main() return bar2(3, 4, 5);"

echo OK
