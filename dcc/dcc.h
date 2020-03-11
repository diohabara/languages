#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
extern Token* token;
// input program
extern char* user_input;

// prototype declaration
void error_at(char* loc, char* fmt, ...);
void error(char* fmt, ...);
bool consume(char* op);
void expect(char* op);
int expect_number(void);
bool at_eof(void);
bool startsWith(char* p, char* q);
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