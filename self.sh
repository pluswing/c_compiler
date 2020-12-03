#!/bin/bash

./9cc 9cc.h tokenize.c parse.c codegen.c main.c | grep -v "^;" > tmp.s
cc -g -static -o 9cc_self tmp.s
./9cc_self 9cc.h tokenize.c parse.c codegen.c main.c | grep -v "^;" > tmp2.s
diff -ur tmp.s tmp2.s

# ./tmp 9cc.h
