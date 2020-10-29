#include "9cc.h"

void gen_val(Node *node) {
  if (node->kind == ND_DEREF) {
    gen(node->lhs);
    return;
  }

  if (node->kind == ND_LVAR) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
  } else if (node->kind == ND_GVAR) {
    printf("  push offset %s\n", node->varname);
  } else if (node->kind == ND_MEMBER) {
    gen_val(node->lhs);
    printf("  pop rax\n");
    printf("  add rax, %d\n", node->member->offset);
    printf("  push rax\n");
  } else {
    error("not VARIABLE");
  }
}

int genCounter = 0;
static char *argreg1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char *argreg2[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static char *argreg4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static char *argreg8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

int breakId = 0;
int continueId = 0;

void gen(Node *node) {
  if (!node) return;
  genCounter += 1;
  int id = genCounter;
  int argCount = 0;
  Type *t;
  int bid = 0;
  int cid = 0;

  switch (node->kind) {
  case ND_STRING:
    printf("  push offset .LC_%d\n", node->string->index);
    return;
  case ND_GVAR_DEF:
    printf("%s:\n", node->varname);
    if (!node->var->init) {
      printf("  .zero %d\n", node->size);
      return;
    }
    if (node->type->ty == ARRAY && node->var->init->block) {
      for (int i = 0; node->var->init->block[i]; i++) {
        switch (node->type->ptr_to->ty) {
        case INT:
          printf("  .long 0x%x\n", node->var->init->block[i]->val);
          break;
        case CHAR:
          printf("  .byte 0x%x\n", node->var->init->block[i]->val);
          break;
        // TODO PTRの場合も考慮する必要あり。
        }
      }
      return;
    }
    if (node->var->init->kind == ND_STRING) {
      if (node->type->ty == ARRAY) {
        printf("  .string \"%s\"\n", node->var->init->string->value);
      } else {
        printf("  .quad .LC_%d\n", node->var->init->string->index);
      }
      return;
    }
    printf("  .long 0x%x\n", node->var->init->val);
    return;
  case ND_ADDR:
    gen_val(node->lhs);
    return;
  case ND_DEREF:
    gen(node->lhs);
    t = get_type(node);
    printf("  pop rax\n");
    if (t && t->ty == CHAR) {
      printf("  movsx rax, BYTE PTR [rax]\n");
    } else if (t && t->ty == INT) {
      printf("  movsxd rax, DWORD PTR [rax]\n");
    } else {
      printf("  mov rax, [rax]\n");
    }
    printf("  push rax\n");
    return;
  case ND_FUNC_DEF:
    printf(".global %s\n", node->funcname);
    printf("%s:\n", node->funcname);
    // プロローグ
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    // 引数の数を除いた変数の数分rspをずらして、変数領域を確保する。
    if (locals[cur_func]) {
      int offset = locals[cur_func]->offset;
      printf("  sub rsp, %d\n", offset);
    }

    // 引数の値をスタックに積む
    for (int i = 0; node->args[i]; i++) {
      if (node->args[i]->size == 1) {
        printf("  mov [rbp-%d], %s\n", node->args[i]->offset, argreg1[i]);
      } else if (node->args[i]->size == 4) {
        printf("  mov [rbp-%d], %s\n", node->args[i]->offset, argreg4[i]);
      } else {
        printf("  mov [rbp-%d], %s\n", node->args[i]->offset, argreg8[i]);
      }
    }

    gen(node->lhs);

    // エピローグ
    // printf("  mov rax, 0");  -> FIXME 必要？？
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_FUNC_CALL:
    for (int i = 0; node->block[i]; i++) {
      gen(node->block[i]);
      argCount++;
    }
    for (int i = argCount - 1; i >= 0; i--) {
      printf("  pop %s\n", argreg8[i]);
    }
    printf("  mov rax, rsp\n");
    printf("  and rax, 15\n");
    printf("  jnz .L.call.%03d\n", id);
    printf("  mov rax, 0\n");
    printf("  call %s\n", node->funcname);
    printf("  jmp .L.end.%03d\n", id);
    printf(".L.call.%03d:\n", id);
    printf("  sub rsp, 8\n");
    printf("  mov rax, 0\n"); // ALに0を入れる。
    printf("  call %s\n", node->funcname);
    printf("  add rsp, 8\n");
    printf(".L.end.%03d:\n", id);
    printf("  push rax\n"); // FIXME あってる？？
    return;
  case ND_BLOCK:
    for (int i = 0; node->block[i]; i++) {
      gen(node->block[i]);
      // TODO 要確認。
      // printf("  pop rax\n");
    }
    return;
  case ND_FOR:
    bid = breakId;
    breakId = id;
    cid = continueId;
    continueId = id;
    // for (A; B; C) {
    //   D;
    // }
    gen(node->lhs->lhs); // A
    printf(".Lbegin%03d:\n", id);
    printf(".Lcontinue%03d:\n", id);
    gen(node->lhs->rhs); // B
    if (!node->lhs->rhs) {
      printf("  push 1\n");
    }
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%03d\n", id);
    gen(node->rhs->lhs); // C
    gen(node->rhs->rhs); // D
    printf("  jmp .Lbegin%03d\n", id);
    printf(".Lend%03d:\n", id);
    breakId = bid;
    continueId = cid;
    return;
  case ND_WHILE:
    bid = breakId;
    breakId = id;
    cid = continueId;
    continueId = id;
    // while (A) {
    //  B;
    // }
    printf(".Lbegin%03d:\n", id);
    printf(".Lcontinue%03d:\n", id);
    gen(node->lhs); // A
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%03d\n", id);
    gen(node->rhs); // B
    printf("  jmp .Lbegin%03d\n", id);
    printf(".Lend%03d:\n", id);
    breakId = bid;
    continueId = cid;
    return;
  case ND_BREAK:
    if (breakId == 0) {
      error("stray break");
    }
    printf("  jmp .Lend%03d\n", breakId);
    return;
  case ND_CONTINUE:
    if (continueId == 0) {
      error("stray continue");
    }
    printf("  jmp .Lcontinue%03d\n", continueId);
    return;
  case ND_IF:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lelse%03d\n", id);
    if (node->rhs->kind == ND_ELSE) {
      gen(node->rhs->lhs);
    } else {
      gen(node->rhs);
    }
    printf("  jmp .Lend%03d\n", id);
    printf(".Lelse%03d:\n", id);
    if (node->rhs->kind == ND_ELSE) {
      gen(node->rhs->rhs);
    }
    printf(".Lend%03d:\n", id);
    return;
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_MEMBER:
  case ND_LVAR:
  case ND_GVAR:
    gen_val(node);
    t = get_type(node);
    if (t && t->ty == ARRAY) {
      return;
    }
    printf("  pop rax\n");
    if (t && t->ty == CHAR) {
      printf("  movsx rax, BYTE PTR [rax]\n");
    } else if (t && t->ty == INT) {
      printf("  movsxd rax, DWORD PTR [rax]\n");
    } else {
      printf("  mov rax, [rax]\n");
    }
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_val(node->lhs);
    gen(node->rhs);
    t = get_type(node);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    if (t && t->ty == CHAR) {
      printf("  mov [rax], dil\n");
    } else if (t && t->ty == INT) {
      printf("  mov [rax], edi\n");
    } else {
      printf("  mov [rax], rdi\n");
    }
    printf("  push rdi\n");
    return;
  case ND_NOT:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    printf("  push rax\n");
    break;
  case ND_BITNOT:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  not rax\n");
    printf("  push rax\n");
    return;
  case ND_LOGAND:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lfalse%03d\n", id);
    gen(node->rhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lfalse%03d\n", id);
    printf("  push 1\n");
    printf("  jmp .Lend%03d\n", id);
    printf(".Lfalse%03d:\n", id);
    printf("  push 0\n");
    printf(".Lend%03d:\n", id);
    return;
  case ND_LOGOR:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  jne .Ltrue%03d\n", id);
    gen(node->rhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  jne .Ltrue%03d\n", id);
    printf("  push 0\n");
    printf("  jmp .Lend%03d\n", id);
    printf(".Ltrue%03d:\n", id);
    printf("  push 1\n");
    printf(".Lend%d:\n", id);
    return;
  case ND_TERNARY:
    // A?B:C
    gen(node->lhs); // A
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lelse%03d\n", id);
    gen(node->rhs->lhs); // B
    printf("  jmp .Lend%03d\n", id);
    printf(".Lelse%03d:\n", id);
    gen(node->rhs->rhs); // C
    printf(".Lend%03d:\n", id);
    return;
  case ND_SWITCH:
    bid = breakId;
    breakId = id;
    node->case_label = id;

    gen(node->lhs);
    printf("  pop rax\n");

    for (Node *n = node->case_next; n; n = n->case_next) {
      genCounter += 1;
      n->case_label = genCounter;
      printf("  cmp rax, %d\n", n->val);
      printf("  je .Lcase%03d\n", n->case_label);
    }

    if (node->default_case) {
      genCounter += 1;
      int i = genCounter;
      node->default_case->case_label = i;
      printf("  jmp .Lcase%03d\n", i);
    }

    printf("  jmp .Lend%03d\n", id);
    gen(node->rhs);
    printf(".Lend%03d:\n", id);
    breakId = bid;
    return;
  case ND_CASE:
    printf(".Lcase%03d:\n", node->case_label);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch(node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_BITAND:
    printf("  and rax, rdi\n");
    break;
  case ND_BITOR:
    printf("  or rax, rdi\n");
    break;
  case ND_BITXOR:
    printf("  xor rax, rdi\n");
    break;
  }

  printf("  push rax\n");
}
