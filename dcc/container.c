#include "dcc.h"

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
// if so return the next Token
Token* consume_ident(char* op) {
  if (token->kind != TK_IDENT || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return NULL;
  return token->next;
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
