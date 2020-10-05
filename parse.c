#include "9cc.h"

LVar *locals[100];
LVar *globals[100];
int cur_func = 0;
StringToken *strings;
int struct_def_index = 0;
Tag *tags;

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

Node *new_node_string(StringToken *s) {
  Node *node = new_node(ND_STRING);
  node->string = s;
  return node;
}

// TODO 100
Node *code[100];


// program    = func*
void program() {
  int i = 0;
  while(!at_eof()) {
    Node *n = func();
    if (!n) {
      continue;
    }
    code[i++] = n;
  }
  code[i] = NULL;
}

// func = "int" ident "(" ("int" ident ("," "int" ident)*)? ")" stmt
Node *func() {
  Node *node;

  if (define_typedef()) {
    return NULL;
  }

  Type *t = define_struct();
  if (t) {
    expect(";");
    return NULL;
  }

  Define *def = read_define();

  if (consume("(")) {
    cur_func++;
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
    node->kind = ND_GVAR_DEF;
    expect(";");
    return node;
  }
}

bool define_typedef() {
  if (!consume_kind(TK_TYPEDEF)) {
    return false;
  }
  Define *def = read_define();
  expect(";");
  push_tag(NULL, def->ident, def->type);
  return true;
}

Type *define_struct() {
  if (!consume_kind(TK_STRUCT)) {
    return NULL;
  }
  Token *name = consume_kind(TK_IDENT);
  if (name && !peek("{")) {
    Tag *tag = find_tag("struct", name);
    if (!tag) {
      error("type not found.");
    }
    return tag->type;
  }
  expect("{");
  Type *t = calloc(1, sizeof(Type));
  t->ty = STRUCT;
  int offset = 0;
  int maxSize = 0;
  while(!consume("}")) {
    Define *def = read_define();
    read_type(def);
    expect(";");
    Member *m = calloc(1, sizeof(Member));
    m->name = calloc(100, sizeof(char));
    memcpy(m->name, def->ident->str, def->ident->len);
    m->ty = def->type;
    int size = get_size(def->type);
    offset = align_to(offset, size);
    m->offset = offset;
    offset += size;
    m->next = t->members;
    t->members = m;
    if (maxSize <= 8 && maxSize < size) {
      maxSize = size;
    }
  }
  t->size = align_to(offset, maxSize);

  if (name) {
    push_tag("struct", name, t);
  }
  return t;
}

// 関数か変数の定義の前半部分を読んで、LVarに詰める
Define *read_define() {
  Type *type = define_struct();
  if (!type) {
    Token *typeToken = consume_kind(TK_TYPE);
    if (!typeToken) {
      return NULL;
    }

    type = calloc(1, sizeof(Type));
    int isChar = memcmp("char", typeToken->str, typeToken->len) == 0;
    type->ty = isChar ? CHAR : INT;
    type->ptr_to = NULL;
  }

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
    node = local_variable_init(node);
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
        int n = node->type->ptr_to->ty == INT ? 4
              : node->type->ptr_to->ty == CHAR ? 1 : 8;
        r = new_binary(ND_MUL, r, new_node_num(n));
      }
      node = new_binary(ND_ADD, node, r);
    } else if (consume("-")) {
      Node *r = mul();
      if (node->type && node->type->ty != INT) {
        int n = node->type->ptr_to->ty == INT ? 4
              : node->type->ptr_to->ty == CHAR ? 1 : 8;
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
    int size = t && t->ty == PTR ? 8
             : t && t->ty == CHAR ? 1 : 4;
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

  if (tok = consume_kind(TK_STRING)) {
    // 文字列
    StringToken *s = calloc(1, sizeof(StringToken));
    s->value = calloc(100, sizeof(char));
    memcpy(s->value, tok->str, tok->len);
    if (strings) {
      s->index = strings->index + 1;
    } else {
      s->index = 0;
    }
    s->next = strings;
    strings = s;
    return new_node_string(s);
  }

  return new_node_num(expect_number());
}

Node *local_variable_init(Node *node) {
  if (!node->var->init) {
    return node;
  }
  Node* assign;

  if (node->var->init->kind == ND_STRING) {
    // 文字列の場合は、各要素をそれぞれ代入する形に変換をかける
    Node* block = calloc(1, sizeof(Node));
    block->block = calloc(100, sizeof(Node));
    block->kind = ND_BLOCK;
    int len = strlen(node->var->init->string->value);
    for (int i = 0; i < node->type->array_size; i++) {
      // add = a[0]
      Node *add = calloc(1, sizeof(Node));
      add->kind = ND_ADD;
      add->lhs = node;
      if (node->type && node->type->ty != INT) {
        int n = node->type->ptr_to->ty == INT ? 4
              : node->type->ptr_to->ty == CHAR ? 1 : 8;
        add->rhs = new_node_num(i * n);
      }
      Node* deref = calloc(1, sizeof(Node));
      deref->kind = ND_DEREF;
      deref->lhs = add;

      assign = calloc(1, sizeof(Node));
      assign->kind = ND_ASSIGN;
      assign->lhs = deref;
      if (len > i) {
        assign->rhs = new_node_num(node->var->init->string->value[i]);
      } else {
        assign->rhs = new_node_num(0);
      }
      block->block[i] = assign;
    }
    return block;
  }

  // int x[] = {1, 2, foo()};
  if (node->type->ty == ARRAY && node->var->init->block) {
    Node* block = calloc(1, sizeof(Node));
    block->block = calloc(100, sizeof(Node));
    block->kind = ND_BLOCK;
    for (int i = 0; node->var->init->block[i]; i++) {
      // add = a[0]
      Node *add = calloc(1, sizeof(Node));
      add->kind = ND_ADD;
      add->lhs = node;
      if (node->type && node->type->ty != INT) {
        int n = node->type->ptr_to->ty == INT ? 4
              : node->type->ptr_to->ty == CHAR ? 1 : 8;
        add->rhs = new_node_num(i * n);
      }
      Node* deref = calloc(1, sizeof(Node));
      deref->kind = ND_DEREF;
      deref->lhs = add;

      assign = calloc(1, sizeof(Node));
      assign->kind = ND_ASSIGN;
      assign->lhs = deref;
      assign->rhs = node->var->init->block[i];
      block->block[i] = assign;
    }
    return block;
  }

  assign = calloc(1, sizeof(Node));
  assign->kind = ND_ASSIGN;
  assign->lhs = node;
  assign->rhs = node->var->init;
  return assign;
}

void read_type(Define *def) {
  if (def == NULL) {
    error("invalid define");
  }
  Type *type = def->type;
  // 配列かチェック
  while (consume("[")) {
    Type *t;
    t = calloc(1, sizeof(Type));
    t->ty = ARRAY;
    t->ptr_to = type;
    t->array_size = 0;
    Token *num = NULL;
    if (num = consume_kind(TK_NUM)) {
      t->array_size = num->val;
    }
    type = t;
    expect("]");
  }
  def->type = type;
}

int get_size(Type *type) {
  if (type->ty == STRUCT) {
    return type->size;
  }
  if (type->ty == ARRAY) {
    if (type->array_size == 0) {
      error("undefined array size.");
    }
    return get_size(type->ptr_to) * type->array_size;
  }
  // TODO sizeをそれぞれ入れておくとこの記述は不要になる。あとでやる
  return type->ty == PTR ? 8 : type->ty == CHAR ? 1 : 4;
}

Node *define_variable(Define *def, LVar **varlist) {
  read_type(def);
  Type *type = def->type;
  Node *node = calloc(1, sizeof(Node));
  node->varname = calloc(100, sizeof(char));
  memcpy(node->varname, def->ident->str, def->ident->len);


  // 初期化式
  /*
  int a = 3;
  char b[] = "foobar";
  int *c = &a;
  char *d = b + 3;
  */
  Node *init = NULL;
  if (consume("=")) {
    // 初期化式あり
    if (consume("{")) {
      // 配列の初期化式
      init = calloc(1, sizeof(Node));
      init->block = calloc(100, sizeof(Node));
      int i = 0;
      for(i = 0; !consume("}"); i++) {
        init->block[i] = expr();
        if (consume("}")) {
          break;
        }
        expect(",");
      }
      if (type->array_size < i) {
        type->array_size = i;
      }
      // TODO 要確認。indexがずれてない？？
      for (i++; i < type->array_size; i++) {
        init->block[i] = new_node_num(0);
      }
    } else {
      init = expr();
      if (init->kind == ND_STRING) {
        int len = strlen(init->string->value) + 1;
        if (type->array_size < len) {
          type->array_size = len;
        }
      }
    }
  }

  node->size = get_size(type);

  LVar *lvar = find_variable(def->ident);
  if (lvar != NULL) {
    error("redefined variable: %s", node->varname);
  }
  // TODO あとでなおす
  node->kind = locals == varlist ? ND_LVAR : ND_GVAR;
  lvar = calloc(1, sizeof(LVar));
  lvar->next = varlist[cur_func];
  lvar->name = def->ident->str;
  lvar->len = def->ident->len;
  lvar->init = init;
  if (varlist[cur_func] == NULL) {
    lvar->offset = node->size;
  } else {
    lvar->offset = varlist[cur_func]->offset + node->size;
  }
  lvar->type = type;
  node->offset = lvar->offset;
  node->type = lvar->type;
  node->var = lvar;
  varlist[cur_func] = lvar;
  return node;
}


// a[3] は *(a + 3)
Node *variable(Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->varname = calloc(100, sizeof(char));
  memcpy(node->varname, tok->str, tok->len);

  LVar *lvar = find_variable(tok);
  if (lvar == NULL) {
    error("undefined variable: %s", node->varname);
  }
  node->kind = lvar->kind == LOCAL ? ND_LVAR : ND_GVAR;
  node->offset = lvar->offset;
  node->type = lvar->type;

  while(true) {
    if (consume("[")) {
      Node *add = calloc(1, sizeof(Node));
      add->kind = ND_ADD;
      add->lhs = node;
      if (node->type && node->type->ty != INT) {
        int n = node->type->ptr_to->ty == INT ? 4
              : node->type->ptr_to->ty == CHAR ? 1 : 8;
        add->rhs = new_binary(ND_MUL, expr(), new_node_num(n));
      }
      node = calloc(1, sizeof(Node));
      node->kind = ND_DEREF;
      node->lhs = add;
      expect("]");
      continue;
    }

    if (consume(".")) {
      node = struct_ref(node);
      continue;
    }

    if (consume("->")) {
      // x->y is short for (*x).y
      Type *t = node->type->ptr_to;
      node = new_binary(ND_DEREF, node, NULL);
      node->type = t;
      node = struct_ref(node);
      continue;
    }
    break;
  }
  return node;
}

Node *struct_ref(Node *node) {
  Node *member = calloc(1, sizeof(Node));
  member->kind = ND_MEMBER;
  member->lhs = node;
  member->member = find_member(consume_kind(TK_IDENT), node->type);
  member->type = member->member->ty;
  return member;
}

Member *find_member(Token *token, Type* type) {
  if (!token) {
    error("member ident not found");
  }
  if (!type) {
    error("member type not found");
  }
  char name[100];
  memcpy(name, token->str, token->len);
  for (Member *m = type->members; m; m = m->next) {
    if (strcmp(name, m->name) == 0) {
      return m;
    }
  }
  error("member not found");
}


LVar *find_variable(Token *tok) {
  for (LVar *var = locals[cur_func]; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      var->kind = LOCAL;
      return var;
    }
  }
  for (LVar *var = globals[0]; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      var->kind = GLOBAL;
      return var;
    }
  }
  return NULL;
}

int align_to(int n, int align) {
  return (n + align - 1) & ~(align - 1);
}

void push_tag(char *prefix, Token *token, Type *type) {
  char *name = calloc(100, sizeof(char));
  if (prefix) {
    memcpy(name, prefix, strlen(prefix));
    memcpy(name + strlen(prefix), " ", 1);
    memcpy(name + strlen(prefix) + 1, token->str, token->len);
  } else {
    memcpy(name, token->str, token->len);
  }
  Tag *tag = calloc(1, sizeof(Tag));
  tag->name = name;
  tag->type = type;
  if (tags) {
    tag->next = tags;
  }
  tags = tag;
}

Tag *find_tag(char *prefix, Token *token) {
  char name[100] = {0};
  if (prefix) {
    memcpy(name, prefix, strlen(prefix));
    memcpy(name + strlen(prefix), " ", 1);
    memcpy(name + strlen(prefix) + 1, token->str, token->len);
  } else {
    memcpy(name, token->str, token->len);
  }
  for (Tag *tag = tags; tag; tag = tag->next) {
    if (strcmp(name, tag->name) == 0) {
      return tag;
    }
  }
  return NULL;
}
