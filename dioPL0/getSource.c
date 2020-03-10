// About input and output

#include "getSource.h"
#include <stdio.h>
#include <string.h>

#define MAXLINE 120
#define MAXERROR 30
#define MAXNUM 14
#define TAB 5
#define INSERT_C "#0000FF"
#define DELETE_C "#FF0000"
#define TYPE_C "#00FF00"

static FILE* fpi;
static FILE* fptex;
static char line[MAXLINE];
static int lineIndex;
static char ch;

static Token cToken;
static KindT idKind;
static int spaces;
static int CR;
static int printed;

static int errorNo = 0;
static char nextChar();
static int isKeySym(KeyId k);
static int isKeyWd(KeyId k);
static void printSpaces();
static void printcToken();

struct keyWd {
    char* word;
    KeyId keyId;
};

static struct keyWd KeyWdT[] = {
    { "begin", Begin },
    { "end", End },
    { "if", If },
    { "then", Then },
    { "while", While },
    { "do", Do },
    { "return", Ret },
    { "function", Func },
    { "var", Var },
    { "const", Const },
    { "odd", Odd },
    { "write", Write },
    { "writeln", WriteLn },
    { "$dummy1", end_of_KeyWd },

    { "+", Plus },
    { "-", Minus },
    { "*", Mult },
    { "/", Div },
    { "%", Remainder },
    { "^", Pow },
    { "(", Lparen },
    { ")", Rparen },
    { "=", Equal },
    { "<", Lss },
    { ">", Gtr },
    { "<>", NotEq },
    { "<=", LssEq },
    { ">=", GtrEq },
    { ",", Comma },
    { ".", Period },
    { ";", Semicolon },
    { ":=", Assign },
    { "$dummy2", end_of_KeySym }
};

int isKeyWd(KeyId k)
{
    return (k < end_of_KeyWd);
}

int isKeySym(KeyId k)
{
    if (k < end_of_KeyWd) {
        return 0;
    }
    return (k < end_of_KeySym);
}

static KeyId charClassT[256];

static void initCharClassT()
{
    int i;
    for (i = 0; i < 256; i++) {
        charClassT[i] = others;
    }
    for (i = '0'; i <= '9'; i++) {
        charClassT[i] = digit;
    }
    for (i = 'A'; i <= 'Z'; i++) {
        charClassT[i] = letter;
    }
    for (i = 'a'; i <= 'z'; i++) {
        charClassT[i] = letter;
    }
    charClassT['+'] = Plus;
    charClassT['-'] = Minus;
    charClassT['*'] = Mult;
    charClassT['/'] = Div;
    charClassT['%'] = Remainder;
    charClassT['^'] = Pow;
    charClassT['('] = Lparen;
    charClassT[')'] = Rparen;
    charClassT['='] = Equal;
    charClassT['<'] = Lss;
    charClassT['>'] = Gtr;
    charClassT[','] = Comma;
    charClassT['.'] = Period;
    charClassT[';'] = Semicolon;
    charClassT[':'] = colon;
}

int openSource(char fileName[])
{
    char fileNameO[30];
    if ((fpi = fopen(fileName, "r")) == NULL) {
        printf("can't open %s\n", fileName);
        return 0;
    }
    strcpy(fileNameO, fileName);
    strcat(fileNameO, ".html");
    if ((fptex = fopen(fileNameO, "w")) == NULL) {
        printf("can't open %s\n", fileNameO);
        return 0;
    }
    return 1;
}

void closeSource()
{
    fclose(fpi);
    fclose(fptex);
}

void initSource()
{
    lineIndex = -1;
    ch = '\n';
    printed = 1;
    initCharClassT();
    fprintf(fptex, "<HTML>\n");
    fprintf(fptex, "<HEAD>\n<TITLE>compiled source program</TITLE>\n</HEAD>\n");
    fprintf(fptex, "<BODY>\n<PRE>\n");
}

void finalSource()
{
    if (cToken.kind == Period) {
        printcToken();
    } else {
        errorInsert(Period);
    }
    fprintf(fptex, "\n</PRE>\n</BODY>\n</HTML>\n");
}

void errorNoCheck()
{
    if (errorNo++ > MAXERROR) {
        fprintf(fptex, "too many errors\n</PRE>\n</BODY>\n</HTML>\n");
        printf("abort compilation\n");
        exit(1);
    }
}

void errorType(char* m)
{
    printSpaces();
    fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
    printcToken();
    errorNoCheck();
}

void errorInsert(KeyId k)
{
    fprintf(fptex, "<FONT COLOR=%s><b>%s</b></FONT>", INSERT_C, KeyWdT[k].word);
    errorNoCheck();
}

void errorMissingId()
{
    fprintf(fptex, "<FONT COLOR=%s>Id</FONT>", INSERT_C);
    errorNoCheck();
}

void errorMissingOp()
{
    fprintf(fptex, "<FONT COLOR=%s>@</FONT>", INSERT_C);
    errorNoCheck();
}

void errorDelete()
{
    int i = (int)cToken.kind;
    printSpaces();
    printed = 1;
    if (i < end_of_KeyWd) {
        fprintf(fptex, "<FONT COLOR=%s><b>%s</b></FONT>", DELETE_C, KeyWdT[i].word);
    } else if (i < end_of_KeySym) {
        fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, KeyWdT[i].word);
    } else if (i == (int)Id) {
        fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, cToken.u.id);
    } else if (i == (int)Num) {
        fprintf(fptex, "<FONT COLOR=%s>%d</FONT>", DELETE_C, cToken.u.value);
    }
}

void errorMessage(char* m)
{
    fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
    errorNoCheck();
}

void errorF(char* m)
{
    errorMessage(m);
    fprintf(fptex, "fatal errors\n</PRE>\n</BODY>\n</HTML>\n");
    if (errorNo) {
        printf("total %d errors\n", errorNo);
    }
    printf("abort compilation\n");
    exit(1);
}

int errorN()
{
    return errorNo;
}

char nextChar()
{
    char ch;
    if (lineIndex == -1) {
        if (fgets(line, MAXLINE, fpi) != NULL) {
            lineIndex = 0;
        } else {
            errorF("end of file\n");
        }
    }
    if ((ch = line[lineIndex++]) == '\n') {
        lineIndex = -1;
        return '\n';
    }
    return ch;
}

Token nextToken()
{
    int i = 0;
    int num;
    KeyId cc;
    Token temp;
    char ident[MAXNAME];
    printcToken();
    spaces = 0;
    CR = 0;
    while (1) {
        if (ch == ' ') {
            spaces++;
        } else if (ch == '\t') {
            spaces += TAB;
        } else if (ch == '\n') {
            spaces = 0;
            CR++;
        } else {
            break;
        }
        ch = nextChar();
    }
    switch (cc = charClassT[ch]) {
    case letter:
        do {
            if (i < MAXNAME) {
                ident[i] = ch;
            }
            i++;
            ch = nextChar();
        } while (charClassT[ch] == letter
            || charClassT[ch] == digit);
        if (i >= MAXNAME) {
            errorMessage("too long");
            i = MAXNAME - 1;
        }
        ident[i] = '\0';
        for (i = 0; i < end_of_KeyWd; i++)
            if (strcmp(ident, KeyWdT[i].word) == 0) {
                temp.kind = KeyWdT[i].keyId;
                cToken = temp;
                printed = 0;
                return temp;
            }
        temp.kind = Id;
        strcpy(temp.u.id, ident);
        break;
    case digit:
        num = 0;
        do {
            num = 10 * num + (ch - '0');
            i++;
            ch = nextChar();
        } while (charClassT[ch] == digit);
        if (i > MAXNUM) {
            errorMessage("too large");
        }
        temp.kind = Num;
        temp.u.value = num;
        break;
    case colon:
        if ((ch = nextChar()) == '=') {
            ch = nextChar();
            temp.kind = Assign;
            break;
        } else {
            temp.kind = nul;
            break;
        }
    case Lss:
        if ((ch = nextChar()) == '=') {
            ch = nextChar();
            temp.kind = LssEq;
            break;
        } else if (ch == '>') {
            ch = nextChar();
            temp.kind = NotEq;
            break;
        } else {
            temp.kind = Lss;
            break;
        }
    case Gtr:
        if ((ch = nextChar()) == '=') {
            ch = nextChar();
            temp.kind = GtrEq;
            break;
        } else {
            temp.kind = Gtr;
            break;
        }
    default:
        temp.kind = cc;
        ch = nextChar();
        break;
    }
    cToken = temp;
    printed = 0;
    return temp;
}

Token checkGet(Token t, KeyId k)
{
    if (t.kind == k) {
        return nextToken();
    }
    if ((isKeyWd(k) && isKeyWd(t.kind)) || (isKeySym(k) && isKeySym(t.kind))) {
        errorDelete();
        errorInsert(k);
        return nextToken();
    }
    errorInsert(k);
    return t;
}

static void printSpaces()
{
    while (spaces-- > 0) {
        fprintf(fptex, " ");
    }
    CR = 0;
    spaces = 0;
}

void printcToken()
{
    int i = (int)cToken.kind;
    if (printed) {
        printed = 0;
        return;
    }
    printed = 1;
    printSpaces();
    if (i < end_of_KeyWd) {
        fprintf(fptex, "<b>%s</b>", KeyWdT[i].word);
    } else if (i < end_of_KeySym) {
        fprintf(fptex, "%s", KeyWdT[i].word);
    } else if (i == (int)Id) {
        switch (idKind) {
        case varId:
            fprintf(fptex, "%s", cToken.u.id);
            return;
        case parId:
            fprintf(fptex, "<i>%s</i>", cToken.u.id);
            return;
        case funcId:
            fprintf(fptex, "<i>%s</i>", cToken.u.id);
            return;
        case constId:
            fprintf(fptex, "<tt>%s</tt>", cToken.u.id);
            return;
        }
    } else if (i == (int)Num)
        fprintf(fptex, "%d", cToken.u.value);
}

void setIdKind(KindT k)
{
    idKind = k;
}
