#!/bin/bash

./9cc 9cc.h tokenize.c parse.c codegen.c main.c | grep -v "^;" > tmp.s
cc -g -static -o 9cc_self tmp.s
# ./tmp 9cc.h
