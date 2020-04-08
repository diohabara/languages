#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// types
typedef struct Token Token;
typedef struct Vector Vector;
typedef struct Node Node;
typedef struct LVar LVar;
typedef struct Function Function;
// types of a token
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
// kinds of a node of an AST
typedef enum {
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_ASSIGN,    // =
  ND_LVAR,      // local variable
  ND_NUM,       // integer
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_RETURN,    // return
  ND_IF,        // if
  ND_IFELSE,    // if else
  ND_WHILE,     // while
  ND_FOR,       // for
  ND_BLOCK,     // {}
  ND_FUNC_CALL  // function call
} NodeKind;
struct Token {
  TokenKind kind;  // type of token
  Token* next;     // next token
  int val;         // if TK_NUM -> the value
  char* str;       // token string
  int len;         // the length of token
};
// vector
struct Vector {
  void** data;
  int capacity;
  int len;
};
// node of AST
struct Node {
  Vector* stmts;    // for ND_BLOCK
  NodeKind kind;    // type of Node
  Node* lhs;        // left hand side
  Node* rhs;        // right hand side
  Node* cond;       // condition
  Node* then;       // then
  Node* els;        // else
  Node* init;       // initilization
  Node* step;       // step
  char* func_name;  // for Function
  int val;          // for ND_NUM
  int offset;       // ND_LVAR
};
// local variables
struct LVar {
  LVar* next;  // the next variable or NULL
  char* name;  // the name of a variable
  int len;     // the length of the name
  int offset;  // offset from RBP
};
// function variables
struct Function {
  Function* next;
  Node* node;
  int len;
  char* name;
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
Vector* new_vector(void);
void vec_push(Vector* vec, void* ele);
void* vec_at(Vector* vec, int index);
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