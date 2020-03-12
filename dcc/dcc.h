#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// types of tokens
typedef enum {
  TK_RESERVED,  // sign
  TK_IDENT,     // identifier
  TK_NUM,       // integer
  TK_EOF,       // end of line
  TK_RETURN,    // return
  TK_IF,        // if
  TK_ELSE,      // else
  TK_WHILE,     // while
  TK_FOR        // for
} TokenKind;
// Token
typedef struct Token Token;
struct Token {
  TokenKind kind;  // type of token
  Token* next;     // next token
  int val;         // if TK_NUM -> the value
  char* str;       // token string
  int len;         // the length of token
};
// a kind of nodes of AST
typedef enum {
  ND_ADD,     // +
  ND_SUB,     // -
  ND_MUL,     // *
  ND_DIV,     // /
  ND_ASSIGN,  // =
  ND_LVAR,    // local variable
  ND_NUM,     // integer
  ND_EQ,      // ==
  ND_NE,      // !=
  ND_LT,      // <
  ND_LE,      // <=
  ND_RETURN,  // return
  ND_IF,      // if
  ND_WHILE,   // while
  ND_FOR      // for
} NodeKind;
// a type of nodes of AST
typedef struct Node Node;
struct Node {
  NodeKind kind;  // type of Node
  Node* lhs;      // left hand side
  Node* rhs;      // right hand side
  int val;        // for ND_NUM
  int offset;     // ND_LVAR
};
// a type of local variables
typedef struct LVar LVar;
struct LVar {
  LVar* next;  // the next variable or NULL
  char* name;  // the name of a variable
  int len;     // the length of the name
  int offset;  // offset from RBP
};
// current token
extern Token* token;
// input program
extern char* user_input;
// array of codes
extern Node* code[100];
// local variable
extern LVar* locals;

// prototype declaration
/// @container.c
void error_at(char* loc, char* fmt, ...);
void error(char* fmt, ...);
bool at_eof(void);
bool is_alnum(char c);
/// @parser.c
bool startsWith(char* p, char* q);
Token* new_token(TokenKind kind, Token* cur, char* str, int len);
Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
Node* new_node_num(int val);
LVar* find_lvar(Token* tok);
bool consume(char* op);
Token* consume_ident(void);
bool consume_return(void);
void expect(char* op);
int expect_number(void);
void tokenize(char* p);
void program(void);
Node* stmt(void);
Node* expr(void);
Node* assign(void);
Node* equality(void);
Node* relational(void);
Node* add(void);
Node* mul(void);
Node* unary(void);
Node* primary(void);
// @codegen.c
void gen_lval(Node* node);
void gen(Node* node);