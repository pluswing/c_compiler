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

  Define *def = read_define();

  if (consume("(")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC_DEF;
    node->funcname = calloc(100, sizeof(char));
    node->args = calloc(10, sizeof(Node*));
    memcpy(node->funcname, def->ident->str, def->ident->len);

    for (int i = 0; !consume(")"); i++) {
      node->args[i] = define_variable(read_define(), locals);
      if (consume(")")) {
        break;
      }
      expect(",");
    }
    node->lhs = stmt();
    return node;
  } else {
    // 変数定義
    node = define_variable(def, globals);
    node->kind = ND_GVAR;
    expect(";");
    return node;
  }
}

// 関数か変数の定義の前半部分を読んで、LVarに詰める
Define *read_define() {
  if (!consume_kind(TK_TYPE)) {
    return NULL;
  }

  Type *type = calloc(1, sizeof(Type));
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
    error("invalid define function or variable");
  }

  Define *def = calloc(1, sizeof(Define));
  def->type = type;
  def->ident = tok;
  return def;
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

  Define *def = read_define();
  if (def) {
    node = define_variable(def, locals);
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
      if (node->type && node->type->ty != INT) {
        int n = node->type->ptr_to->ty == INT ? 4 : 8;
        r = new_binary(ND_MUL, r, new_node_num(n));
      }
      node = new_binary(ND_ADD, node, r);
    } else if (consume("-")) {
      Node *r = mul();
      if (node->type && node->type->ty != INT) {
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

Node *define_variable(Define *def, LVar **varlist) {
  if (def == NULL) {
    error("invalid define");
  }
  Type *type = def->type;

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
  node->varname = calloc(100, sizeof(char));
  memcpy(node->varname, def->ident->str, def->ident->len);
  node->kind = ND_LVAR;
  node->size = size;

  LVar *lvar = find_lvar(def->ident);
  if (lvar != NULL) {
    error("redefined variable: %s", node->varname);
  }
  lvar = calloc(1, sizeof(LVar));
  lvar->next = varlist[cur_func];
  lvar->name = def->ident->str;
  lvar->len = def->ident->len;
  if (varlist[cur_func] == NULL) {
    lvar->offset = size;
  } else {
    lvar->offset = varlist[cur_func]->offset + size;
  }
  lvar->type = type;
  node->offset = lvar->offset;
  node->type = lvar->type;
  varlist[cur_func] = lvar;
  return node;
}


// a[3] は *(a + 3)
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

  while (consume("[")) {
    Node *add = calloc(1, sizeof(Node));
    add->kind = ND_ADD;
    add->lhs = node;
    add->rhs = expr();
    node = calloc(1, sizeof(Node));
    node->kind = ND_DEREF;
    node->lhs = add;
    expect("]");
  }
  return node;
}
