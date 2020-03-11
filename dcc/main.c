#include "dcc.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    error("the number of arguments is supposed to be ONE");
    return 1;
  }

  // tokenize and parse
  user_input = argv[1];
  tokenize(user_input);
  program();
  // token = tokenize(user_input);
  // Node* node = program();

  // the first part of assembly
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  // generate code through AST
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // as a result of a expression, one value is inside a stack
    // so, pop so as not to overflow
    printf("  pop rax\n");
  }

  // epilogue
  // the last result of the expression is in RAX
  printf("  mov rsp, rbp\n");
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
