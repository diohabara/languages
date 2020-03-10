#include <stdio.h>
#ifndef TBL
#define TBL
#include "table.h"
#endif

#define MAXNAME 31 // the maximum length of variables

typedef enum keys {
    Begin,
    End,
    If,
    Then,
    While,
    Do,
    Ret,
    Func,
    Var,
    Const,
    Odd,
    Write,
    WriteLn,
    end_of_KeyWd,
    Plus,
    Minus,
    Mult,
    Div,
    Remainder,
    Pow,
    Lparen,
    Rparen,
    Equal,
    Lss,
    Gtr,
    NotEq,
    LssEq,
    GtrEq,
    Comma,
    Period,
    Semicolon,
    Assign,
    end_of_KeySym,
    Id,
    Num,
    nul,
    end_of_Token,
    letter,
    digit,
    colon,
    others
} KeyId;

typedef struct token {
    KeyId kind;
    union {
        char id[MAXNAME];
        int value;
    } u;
} Token;

Token nextToken();
Token checkGet(Token t, KeyId k);

int openSource(char fileName[]);
void closeSource();
void initSource();
void finalSource();
void errorType(char* m);
void errorInsert(KeyId k);
void errorMissingId();
void errorMissingOp();
void errorDelete();
void errorMessage(char* m);
void errorF(char* m);
int errorN();

void setIdKind(KindT k);
