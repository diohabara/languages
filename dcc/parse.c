#include "dcc.h"

/** Grammar
 * program    = stmt*
 * stmt       = expr ";"
 * stmt       = expr ";"
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "while" "(" expr ")" stmt
              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
              | "return" expr ";"
 * expr       = assign
 * assign     = equality ("=" assign)?
 * equality   = relational ("==" relational | "!=" relational)*
 * relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 * add        = mul ("+" mul | "-" mul)*
 * mul        = unary ("*" unary | "/" unary)*
 * unary      = ("+" | "-")? primary
 * primary    = num | ident | "(" expr ")"
 **/

Token* token;
Node* code[100];
LVar* locals;
int i = 0;

// make a token connected to cur
Token* new_token(TokenKind kind, Token* cur, char* str, int len) {
  Token* tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

Node* new_node(NodeKind kind, Node* lhs, Node* rhs) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node* new_node_num(int val) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

bool startsWith(char* p, char* q) { return memcmp(p, q, strlen(q)) == 0; }

// read a token if the next token is an expected one
// if so return true otherwise false
bool consume(char* op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;

  token = token->next;
  return true;
}

// read a token if the next token is an expected one
// if so return the next Token
// otherwise, return NULL
Token* consume_ident(void) {
  if (token->kind != TK_IDENT) return NULL;
  Token* cur = token;
  token = token->next;
  return cur;
}

// read a token if the next token is "ND_RETURN"
bool consume_return(void) {
  if (token->kind != TK_RETURN) return false;
  token = token->next;
  return true;
}

// read a token if the next token is "ND_IF"
bool consume_if(void) {
  if (token->kind != TK_IF) return false;
  token = token->next;
  return true;
}

bool consume_else(void) {
  if (token->kind != TK_ELSE) return false;
  token = token->next;
  return true;
}

// read a token if the next token is "ND_WHILE"
bool consume_while(void) {
  if (token->kind != TK_WHILE) return false;
  token = token->next;
  return true;
}

// read a token if the next token is "ND_FOR"
bool consume_for(void) {
  if (token->kind != TK_FOR) return false;
  token = token->next;
  return true;
}

// read a token if the next token is an expected one
// otherwise alert error
void expect(char* op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error("%s is not %s", token->str, op);
  token = token->next;
}

// read a token and return the number if the next token is number
// otherwise alert error
int expect_number(void) {
  if (token->kind != TK_NUM) error("%s is not number", token->str);

  int val = token->val;
  token = token->next;
  return val;
}

LVar* find_lvar(Token* tok) {
  for (LVar* var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

// tokenize input string p and return it
void tokenize(char* p) {
  Token head;
  head.next = NULL;
  Token* cur = &head;

  while (*p) {
    // skip space
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (startsWith(p, "if")) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }

    if (startsWith(p, "else")) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    if (startsWith(p, "while")) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    if (startsWith(p, "for")) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }

    if (startsWith(p, "==") || startsWith(p, "!=") || startsWith(p, "<=") ||
        startsWith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()<>;=", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char* q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      int len = 1;
      char* start = p;
      for (;;) {
        p++;
        if ('a' <= *p && *p <= 'z')
          len++;
        else
          break;
      }
      char* dst = (char*)malloc(sizeof(char) * sizeof(len));
      strncpy(dst, start, len);
      dst[len] = '\0';
      cur = new_token(TK_IDENT, cur, dst, len);
      continue;
    }

    error("%s cannot be tokenized", p);
  }

  cur = new_token(TK_EOF, cur, p, 0);
  token = head.next;
}

// program = stmt*
void program(void) {
  int i = 0;
  while (!at_eof()) code[i++] = stmt();
  code[i] = NULL;
}

// stmt = expr ";" | "return" expr ";"
Node* stmt(void) {
  Node* node;
  if (consume_return()) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }
  expect(";");
  return node;
}

// expr = assign
Node* expr(void) { return assign(); }

// assign = equality ("=" assign)?
Node* assign(void) {
  Node* node = equality();
  if (consume("=")) node = new_node(ND_ASSIGN, node, assign());
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node* equality(void) {
  Node* node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node* relational(void) {
  Node* node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node* add(void) {
  Node* node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul  = unary ("*" unary | "/" unary)
Node* mul(void) {
  Node* node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = ("+" | "-")? primary
Node* unary(void) {
  if (consume("+")) return unary();
  if (consume("-")) return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

// primary = num | ident | "(" expr ")"
Node* primary(void) {
  Token* tok = consume_ident();
  if (tok) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar* lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals->offset + 8;
      node->offset = lvar->offset;
      locals = lvar;
    }
    return node;
  }

  if (consume("(")) {
    Node* node = expr();
    expect(")");
    return node;
  }

  return new_node_num(expect_number());
}