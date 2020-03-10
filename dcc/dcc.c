#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Grammar
 *expr       = equality
 *equality   = relational ("==" *relational | "!=" relational)*
 *relational = add ("<" add | "<=" add *| ">" add | ">=" add)*
 *add        = mul ("+" mul | "-" mul)*
 *mul        = unary ("*" unary | "/" unary)*
 *unary      = ("+" | "-")? primary
 *primary    = num | "(" expr ")"
 **/

// types of tokens
typedef enum {
  TK_RESERVED,  // sign
  TK_NUM,       // integer
  TK_EOF,       // end of line
} TokenKind;
typedef struct Token Token;
typedef struct Node Node;
// Token
struct Token {
  TokenKind kind;  // type of token
  Token* next;     // next token
  int val;         // if TK_NUM -> the value
  char* str;       // token string
  int len;         // the length of token
};
// kinds of nodes of AST
typedef enum {
  ND_ADD,  // +
  ND_SUB,  // -
  ND_MUL,  // *
  ND_DIV,  // /
  ND_NUM,  // integer
  ND_EQ,   // ==
  ND_NE,   // !=
  ND_LT,   // <
  ND_LE,   // <=
} NodeKind;
// types of nodes of AST
struct Node {
  NodeKind kind;  // type of Node
  Node* lhs;      // left hand side
  Node* rhs;      // right hand side
  int val;
};
// current token
Token* token;
// input program
char* user_input;

// prototype declaration
void error_at(char* loc, char* fmt, ...);
void error(char* fmt, ...);
bool consume(char* op);
void expect(char* op);
int expect_number(void);
bool at_eof(void);
Token* new_token(TokenKind kind, Token* cur, char* str, int len);
Token* tokenize(char* p);
Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
Node* new_node_num(int val);
Node* expr(void);
Node* equality(void);
Node* relational(void);
Node* add(void);
Node* mul(void);
Node* unary(void);
Node* primary(void);

void gen(Node* node);
bool startsWith(char* p, char* q);

int main(int argc, char** argv) {
  if (argc != 2) {
    error("the number of arguments is supposed to be ONE");
    return 1;
  }

  // tokenize and parse
  user_input = argv[1];
  token = tokenize(user_input);
  Node* node = expr();

  // the first part of assembly
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // generate code through AST
  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}

// report where the error is
void error_at(char* loc, char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");  // output spaces
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  // fprintf(stderr, "%s\n", loc);
  exit(1);
}

// alert error
// same argument as printf
void error(char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

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
// otherwise alert error
void expect(char* op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "is not %s", op);
  token = token->next;
}

// read a token and return the number if the next token is number
// otherwise alert error
int expect_number(void) {
  if (token->kind != TK_NUM) error_at(token->str, "is not number");

  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof(void) { return token->kind == TK_EOF; }

// make a token connected to cur
Token* new_token(TokenKind kind, Token* cur, char* str, int len) {
  Token* tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startsWith(char* p, char* q) { return memcmp(p, q, strlen(q)) == 0; }

// tokenize input string p and return it
Token* tokenize(char* p) {
  Token head;
  head.next = NULL;
  Token* cur = &head;

  while (*p) {
    // skip space
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (startsWith(p, "==") || startsWith(p, "!=") || startsWith(p, "<=") ||
        startsWith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()<>", *p)) {
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
    error_at(p, "cannot be tokenized");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
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

// expr = equality
Node* expr(void) { return equality(); }

// epuality = relational (== relational | != relational)*
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

// relational = add (< add | <= add | > add | >= add)*
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

// add = mul (+ mul | - mul)*
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

// mul (+ mul | - mul)*
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

// (+ | -)? primary
Node* unary(void) {
  if (consume("+")) return unary();
  if (consume("-")) return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

// num | ( expr )
Node* primary(void) {
  if (consume("(")) {
    Node* node = expr();
    expect(")");
    return node;
  }

  return new_node_num(expect_number());
}

void gen(Node* node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
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
  }

  printf("  push rax\n");
}
