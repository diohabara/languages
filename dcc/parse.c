#include "dcc.h"

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

// read the next token if it is an expected one
// if so return true otherwise false
bool consume(char* op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;

  token = token->next;
  return true;
}

// read the next token if the it is "TK_IDENT"
// if so, return it
// otherwise, return NULL
Token* consume_ident(void) {
  if (token->kind != TK_IDENT) return NULL;
  Token* cur = token;
  token = token->next;
  return cur;
}

// read the next token if it is "ND_RETURN"
bool consume_return(void) {
  if (token->kind != TK_RETURN) return false;
  token = token->next;
  return true;
}

// read the next token if it is "TK_IF"
bool consume_if(void) {
  if (token->kind != TK_IF) return false;
  token = token->next;
  return true;
}

// read the next token if it is "TK_ELSE"
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
    error("***\n%s is expected, but got %s\n***", op, token->str);
  token = token->next;
}

// read a token and return the number if the next token is number
// otherwise alert error
int expect_number(void) {
  if (token->kind != TK_NUM) error("***\n%s is not number\n***", token->str);

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

Function* find_func(Token* tok) {
  for (Function* var = locals; var; var = var->next)
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
    // return
    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }
    // if
    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }
    // else
    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }
    // while
    if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }
    // for
    if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }
    // relational
    if (startsWith(p, "==") || startsWith(p, "!=") || startsWith(p, "<=") ||
        startsWith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }
    // others
    if (strchr("+-*/()<>;={}", *p)) {
      cur = new_token(TK_RESERVED, cur, p, 1);
      ++p;
      continue;
    }
    // num
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char* q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }
    // variables
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

/* stmt = expr ";"
        | "{" stmt* "}"
        | "if" "(" expr ")" stmt ("else" stmt)?
        | "while" "(" expr ")" stmt
        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
        | "return" expr ";"
*/
Node* stmt(void) {
  Node* node;
  if (consume("{")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->stmts = new_vector();
    while (!consume("}")) {
      vec_push(node->stmts, stmt());
    }
    return node;
  } else if (consume_if()) {
    node = calloc(1, sizeof(Node));
    if (consume("(")) {
      node->cond = expr();
      expect(")");
      node->then = stmt();
      if (consume_else()) {
        node->kind = ND_IFELSE;
        node->els = stmt();
      } else {
        node->kind = ND_IF;
      }
      return node;
    }
  } else if (consume_while()) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    if (consume("(")) {
      node->cond = expr();
      expect(")");
      node->then = stmt();
      return node;
    }
  } else if (consume_for()) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    if (consume("(")) {
      node->init = expr();
      expect(";");
      node->cond = expr();
      expect(";");
      node->step = expr();
      expect(")");
      node->lhs = stmt();
      return node;
    }
  } else if (consume_return()) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else if (consume("{")) {
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

/** primary = num
 *          | ident ("(" ")")?
 *          | "(" expr ")"
 */
Node* primary(void) {
  if (consume("(")) {
    Node* node = expr();
    expect(")");
    return node;
  }

  Token* tok = consume_ident();
  if (!tok) return new_node_num(expect_number());

  Node* node;
  if (consume("(")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC_CALL;
    expect(")");
    return node;
  }

  node = calloc(1, sizeof(Node));
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