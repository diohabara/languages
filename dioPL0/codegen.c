// The subroutine of code generation and the routine of execution of code

#include "codegen.h"
#include <stdio.h>
#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "getSource.h"

#define MAXCODE 200
#define MAXMEM 2000
#define MAXREG 20
#define MAXLEVEL 5

typedef struct inst {
    OpCode opCode;
    union {
        RelAddr addr;
        int value;
        Operator optr;
    } u;
} Inst;

static Inst code[MAXCODE];
static int cIndex = -1;
static void checkMax();
static void printCode(int i);

int nextCode()
{
    return cIndex + 1;
}

int genCodeV(OpCode op, int v)
{
    checkMax();
    code[cIndex].opCode = op;
    code[cIndex].u.value = v;
    return cIndex;
}

int genCodeT(OpCode op, int ti)
{
    checkMax();
    code[cIndex].opCode = op;
    code[cIndex].u.addr = relAddr(ti);
    return cIndex;
}

int genCodeO(Operator p)
{
    checkMax();
    code[cIndex].opCode = opr;
    code[cIndex].u.optr = p;
    return cIndex;
}

int genCodeR()
{
    if (code[cIndex].opCode == ret) {
        return cIndex;
    }
    checkMax();
    code[cIndex].opCode = ret;
    code[cIndex].u.addr.level = bLevel();
    code[cIndex].u.addr.addr = fPars();
    return cIndex;
}

void checkMax()
{
    if (++cIndex < MAXCODE) {
        return;
    }
    errorF("too many code");
}

void backPatch(int i)
{
    code[i].u.value = cIndex + 1;
}

void listCode()
{
    int i;
    printf("\ncode\n");
    for (i = 0; i <= cIndex; i++) {
        printf("%3d: ", i);
        printCode(i);
    }
}

void printCode(int i)
{
    int flag;
    switch (code[i].opCode) {
    case lit:
        printf("lit");
        flag = 1;
        break;
    case opr:
        printf("opr");
        flag = 3;
        break;
    case lod:
        printf("lod");
        flag = 2;
        break;
    case sto:
        printf("sto");
        flag = 2;
        break;
    case cal:
        printf("cal");
        flag = 2;
        break;
    case ret:
        printf("ret");
        flag = 2;
        break;
    case ict:
        printf("ict");
        flag = 1;
        break;
    case jmp:
        printf("jmp");
        flag = 1;
        break;
    case jpc:
        printf("jpc");
        flag = 1;
        break;
    }
    switch (flag) {
    case 1:
        printf(",%d\n", code[i].u.value);
        return;
    case 2:
        printf(",%d", code[i].u.addr.level);
        printf(",%d\n", code[i].u.addr.addr);
        return;
    case 3:
        switch (code[i].u.optr) {
        case neg:
            printf(",neg\n");
            return;
        case add:
            printf(",add\n");
            return;
        case sub:
            printf(",sub\n");
            return;
        case mul:
            printf(",mul\n");
            return;
        case div:
            printf(",div\n");
            return;
        case remainder:
            printf(",remainder\n");
            return;
        case pow:
            printf(",pow\n");
            return;
        case odd:
            printf(",odd\n");
            return;
        case eq:
            printf(",eq\n");
            return;
        case ls:
            printf(",ls\n");
            return;
        case gr:
            printf(",gr\n");
            return;
        case neq:
            printf(",neq\n");
            return;
        case lseq:
            printf(",lseq\n");
            return;
        case greq:
            printf(",greq\n");
            return;
        case wrt:
            printf(",wrt\n");
            return;
        case wrl:
            printf(",wrl\n");
            return;
        }
    }
}

void execute()
{
    int stack[MAXMEM];
    int display[MAXLEVEL];
    int pc, top, lev, temp;
    Inst i;
    printf("start execution\n");
    top = 0;
    pc = 0;
    stack[0] = 0;
    stack[1] = 0;
    display[0] = 0;
    do {
        i = code[pc++];
        switch (i.opCode) {
        case lit:
            stack[top++] = i.u.value;
            break;
        case lod:
            stack[top++] = stack[display[i.u.addr.level] + i.u.addr.addr];
            break;
        case sto:
            stack[display[i.u.addr.level] + i.u.addr.addr] = stack[--top];
            break;
        case cal:
            lev = i.u.addr.level + 1;
            stack[top] = display[lev];
            stack[top + 1] = pc;
            display[lev] = top;
            pc = i.u.addr.addr;
            break;
        case ret:
            temp = stack[--top];
            top = display[i.u.addr.level];
            display[i.u.addr.level] = stack[top];
            pc = stack[top + 1];
            top -= i.u.addr.addr;
            stack[top++] = temp;
            break;
        case ict:
            top += i.u.value;
            if (top >= MAXMEM - MAXREG)
                errorF("stack overflow");
            break;
        case jmp:
            pc = i.u.value;
            break;
        case jpc:
            if (stack[--top] == 0) {
                pc = i.u.value;
            }
            break;
        case opr:
            switch (i.u.optr) {
            case neg:
                stack[top - 1] = -stack[top - 1];
                continue;
            case add:
                --top;
                stack[top - 1] += stack[top];
                continue;
            case sub:
                --top;
                stack[top - 1] -= stack[top];
                continue;
            case mul:
                --top;
                stack[top - 1] *= stack[top];
                continue;
            case div:
                --top;
                stack[top - 1] /= stack[top];
                continue;
            case remainder:
                --top;
                stack[top - 1] %= stack[top];
                continue;
            case pow:
                --top;
                int base = stack[top];
                int exp = stack[top - 1];
                if (base == 0) {
                    stack[top - 1] = 1;
                } else {
                    for (int i = 0; i < base - 1; i++) {
                        stack[top - 1] *= exp;
                    }
                }
                continue;
            case odd:
                stack[top - 1] = stack[top - 1] & 1;
                continue;
            case eq:
                --top;
                stack[top - 1] = (stack[top - 1] == stack[top]);
                continue;
            case ls:
                --top;
                stack[top - 1] = (stack[top - 1] < stack[top]);
                continue;
            case gr:
                --top;
                stack[top - 1] = (stack[top - 1] > stack[top]);
                continue;
            case neq:
                --top;
                stack[top - 1] = (stack[top - 1] != stack[top]);
                continue;
            case lseq:
                --top;
                stack[top - 1] = (stack[top - 1] <= stack[top]);
                continue;
            case greq:
                --top;
                stack[top - 1] = (stack[top - 1] >= stack[top]);
                continue;
            case wrt:
                printf("%d", stack[--top]);
                continue;
            case wrl:
                printf("\n");
                continue;
            }
        }
    } while (pc != 0);
}
