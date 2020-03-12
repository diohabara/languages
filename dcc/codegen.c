#include "dcc.h"

void gen_lval(Node* node) {
  if (node->kind != ND_LVAR)
    error("the left hand side of the assignment is not a variable");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node* node) {
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    case ND_IF:        // if (cond) then
      // cond
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .LendXXX\n");
      // then
      printf(".LendXXX:");
      return;
    case ND_IFELSE:  // if (cond) then else els
      printf(".LbeginXXX:");
      // cond
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .LendXXX");
      // then
      printf("  jmp .LendXXX");
      printf(".LelseXXX");
      // els
      printf(".LendXXX");
      return;
    case ND_WHILE: // while (cond) then
      printf(".LbeginXXX:");
      // cond
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .LendXXX\n");
      // then
      printf("  jmp .LbeginXXX\n");
      printf(".LendXXX:");
      return;
    case ND_FOR:  // for (init; cond; step) then;
      // init
      printf(".LbeginXXX:\n");
      // cond
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .LendXXX\n");
      // step
      // then
      printf("  je  .LbeginXXX\n");
      printf(".LendXXX:\n");
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
