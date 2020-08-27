#!/bin/bash

runTest() {
  ./9cc "tests/test.c" > tmp.s
  cd func
  cc -c func.c
  cd ..
  cc -static -o tmp tmp.s func/func.o
  ./tmp
}

runTest
