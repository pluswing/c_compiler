#include "9cc.h"

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

// TODO 100
Node *code[100];


// program    = func*
void program() {
  int i = 0;
  while(!at_eof()) {
    code[i++] = func();
  }
  code[i] = NULL;
}

// func = "int" ident "(" ("int" ident ("," "int" ident)*)? ")" stmt
Node *func() {
  cur_func++;
  Node *node;
  if (!consume_kind(TK_TYPE)) {
    error("function retun type not found.");
  }

  Token *tok = consume_kind(TK_IDENT);
  if (tok == NULL) {
    error("not function!");
  }
  node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC_DEF;
  node->funcname = calloc(100, sizeof(char));
  node->args = calloc(10, sizeof(Node*));
  memcpy(node->funcname, tok->str, tok->len);
  expect("(");

  for (int i = 0; !consume(")"); i++) {
    if (!consume_kind(TK_TYPE)) {
      error("function args type not found.");
    }
    node->args[i] = define_variable();
    if (consume(")")) {
      break;
    }
    expect(",");
  }
  node->lhs = stmt();
  return node;
}

// stmt    = expr ";"
//        | "{" stmt* "}"
//        | "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt)?
//        | "while" "(" expr ")" stmt
//        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//        | "int" "*"* ident ";"
Node *stmt() {
  Node *node;
  if (consume("{")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    // TODO 100
    node->block = calloc(100, sizeof(Node));
    for(int i = 0; !consume("}"); i++) {
      node->block[i] = stmt();
    }
    return node;
  }

  if (consume_kind(TK_FOR)) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;

    Node *left = calloc(1, sizeof(Node));
    left->kind = ND_FOR_LEFT;
    Node *right = calloc(1, sizeof(Node));
    right->kind = ND_FOR_RIGHT;

    if (!consume(";")) {
      left->lhs = expr();
      expect(";");
    }

    if (!consume(";")) {
      left->rhs = expr();
      expect(";");
    }

    if (!consume(")")) {
      right->lhs = expr();
      expect(")");
    }
    right->rhs = stmt();

    node->lhs = left;
    node->rhs = right;
    return node;
  }

  if (consume_kind(TK_WHILE)) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
    return node;
  }

  if (consume_kind(TK_IF)) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
    if (consume_kind(TK_ELSE)) {
      Node *els = calloc(1, sizeof(Node));
      els->kind = ND_ELSE;
      els->lhs = node->rhs;
      els->rhs = stmt();
      node->rhs = els;
    }
    return node;
  }

  if (consume_kind(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
    return node;
  }

  if (consume_kind(TK_TYPE)) {
    node = define_variable();
    expect(";");
    return node;
  }

  node = expr();
  expect(";");
  return node;
}

// expr       = assign
Node *expr() {
  return assign();
}

// assign     = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    node = new_binary(ND_ASSIGN, node, assign());
  }
  return node;
}

// equality   = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();
  for(;;) {
    if (consume("==")) {
      node = new_binary(ND_EQ, node, relational());
    } else if (consume("!=")) {
      node = new_binary(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();
  for (;;) {
    if (consume("<")) {
      node = new_binary(ND_LT, node, add());
    } else if (consume("<=")) {
      node = new_binary(ND_LE, node, add());
    } else if (consume(">")) {
      node = new_binary(ND_LT, add(), node);
    } else if (consume(">=")) {
      node = new_binary(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

// add        = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();
  for (;;) {
    if (consume("+")) {
      Node *r = mul();
      if (node->type && node->type->ty == PTR) {
        int n = node->type->ptr_to->ty == INT ? 4 : 8;
        r = new_binary(ND_MUL, r, new_node_num(n));
      }
      node = new_binary(ND_ADD, node, r);
    } else if (consume("-")) {
      Node *r = mul();
      if (node->type && node->type->ty == PTR) {
        int n = node->type->ptr_to->ty == INT ? 4 : 8;
        r = new_binary(ND_MUL, r, new_node_num(n));
      }
      node = new_binary(ND_SUB, node, r);
    } else {
      return node;
    }
  }
}

// mul        = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();
  for (;;) {
    if (consume("*")) {
      node = new_binary(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_binary(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

// unary = "sizeof" unary
//       | "+"? primary
//       | "-"? primary
//       | "*" unary
//       | "&" unary
Node *unary() {
  if (consume("+")) {
    return unary();
  }
  if (consume("-")) {
    return new_binary(ND_SUB, new_node_num(0), unary());
  }
  if (consume("*")) {
    return new_binary(ND_DEREF, unary(), NULL);
  }
  if (consume("&")) {
    return new_binary(ND_ADDR, unary(), NULL);
  }
  if (consume_kind(TK_SIZEOF)) {
    Node *n = unary();
    Type *t = get_type(n);
    int size = t && t->ty == PTR ? 8 : 4;
    return new_node_num(size);
  }
  return primary();
}

Type *get_type(Node *node) {
  if (node == NULL) {
    return NULL;
  }
  if (node->type) {
    return node->type;
  }

  Type *t = get_type(node->lhs);
  if (t == NULL) {
    t = get_type(node->rhs);
  }

  if (t && node->kind == ND_DEREF) {
    t = t->ptr_to;
    if (t == NULL) {
      error("invalid dereference");
    }
    return t;
  }
  return t;
}

// primary = num
//        | ident ("(" expr* ")")?
//        | "(" expr ")"
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_kind(TK_IDENT);
  if (tok) {
    if (consume("(")) {
      // 関数呼び出し
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_FUNC_CALL;
      node->funcname = calloc(100, sizeof(char));
      memcpy(node->funcname, tok->str, tok->len);
      // 引数 TODO とりあえず10個まで。
      node->block = calloc(10, sizeof(Node));
      for(int i = 0; !consume(")"); i++) {
        node->block[i] = expr();
        if (consume(")")) {
          break;
        }
        expect(",");
      }
      return node;
    }
    // 関数呼び出しではない場合、変数。
    return variable(tok);
  }

  return new_node_num(expect_number());
}

Node *define_variable() {

  Type *type;
  type = calloc(1, sizeof(Type));
  type->ty = INT;
  type->ptr_to = NULL;
  while(consume("*")) {
    Type *t;
    t = calloc(1, sizeof(Type));
    t->ty = PTR;
    t->ptr_to = type;
    type = t;
  }

  Token *tok = consume_kind(TK_IDENT);
  if (tok == NULL) {
    error("invalid define variable");
  }

  int size = type->ty == PTR ? 8 : 4;

  // 配列かチェック
  while (consume("[")) {
    Type *t;
    t = calloc(1, sizeof(Type));
    t->ty = ARRAY;
    t->ptr_to = type;
    t->array_size = expect_number();
    type = t;
    size *= t->array_size;
    expect("]");
  }

  // TODO 要確認 変数のoffset値は8の倍数じゃないとダメっぽい。
  while((size % 8) != 0) {
    size += 4;
  }

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

  LVar *lvar = find_lvar(tok);
  if (lvar != NULL) {
    char name[100] = {0};
    memcpy(name, tok->str, tok->len);
    error("redefined variable: %s", name);
  }
  lvar = calloc(1, sizeof(LVar));
  lvar->next = locals[cur_func];
  lvar->name = tok->str;
  lvar->len = tok->len;
  if (locals[cur_func] == NULL) {
    lvar->offset = size;
  } else {
    lvar->offset = locals[cur_func]->offset + size;
  }
  lvar->type = type;
  node->offset = lvar->offset;
  node->type = lvar->type;
  locals[cur_func] = lvar;
  return node;
}


Node *variable(Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

  LVar *lvar = find_lvar(tok);
  if (lvar == NULL) {
    char name[100] = {0};
    memcpy(name, tok->str, tok->len);
    error("undefined variable: %s", name);
  }
  node->offset = lvar->offset;
  node->type = lvar->type;
  return node;
}
