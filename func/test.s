.intel_syntax noprefix
.globl main
main:
  push 3
  push 4
  pop rsi
  pop rdi
  call bar
