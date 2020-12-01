#!/bin/bash

./9cc 9cc.h tokenize.c parse.c codegen.c main.c > tmp.s
cc -g -static -o tmp tmp.s
# ./tmp 9cc.h
