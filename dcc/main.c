#include "dcc.h"

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
