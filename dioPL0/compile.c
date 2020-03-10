// The main routine of parsing and code generation

#include "getSource.h"
#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "codegen.h"

#define MINERROR 3
#define FIRSTADDR 2

static Token token;

static void block(int pIndex);
static void constDecl();
static void varDecl();
static void funcDecl();
static void statement();
static void expression();
static void term();
static void factor();
static void condition();
static int isStBeginKey(Token t);

int compile()
{
    int i;
    printf("start compilation\n");
    initSource();
    token = nextToken();
    blockBegin(FIRSTADDR);
    block(0);
    finalSource();
    i = errorN();
    if (i != 0) {
        printf("%d errors\n", i);
    }
    return i < MINERROR;
}

void block(int pIndex)
{
    int backP;
    backP = genCodeV(jmp, 0);
    while (1) {
        switch (token.kind) {
        case Const:
            token = nextToken();
            constDecl();
            continue;
        case Var:
            token = nextToken();
            varDecl();
            continue;
        case Func:
            token = nextToken();
            funcDecl();
            continue;
        default:
            break;
        }
        break;
    }
    backPatch(backP);
    changeV(pIndex, nextCode());
    genCodeV(ict, frameL());
    statement();
    genCodeR();
    blockEnd();
}

void constDecl()
{
    Token temp;
    while (1) {
        if (token.kind == Id) {
            setIdKind(constId);
            temp = token;
            token = checkGet(nextToken(), Equal);
            if (token.kind == Num) {
                enterTconst(temp.u.id, token.u.value);
            } else {
                errorType("number");
            }
            token = nextToken();
        } else {
            errorMissingId();
        }
        if (token.kind != Comma) {
            if (token.kind == Id) {
                errorInsert(Comma);
                continue;
            } else
                break;
        }
        token = nextToken();
    }
    token = checkGet(token, Semicolon);
}

void varDecl()
{
    while (1) {
        if (token.kind == Id) {
            setIdKind(varId);
            enterTvar(token.u.id);
            token = nextToken();
        } else {
            errorMissingId();
        }
        if (token.kind != Comma) {
            if (token.kind == Id) {
                errorInsert(Comma);
                continue;
            } else {
                break;
            }
        }
        token = nextToken();
    }
    token = checkGet(token, Semicolon);
}

void funcDecl()
{
    int fIndex;
    if (token.kind == Id) {
        setIdKind(funcId);
        fIndex = enterTfunc(token.u.id, nextCode());
        token = checkGet(nextToken(), Lparen);
        blockBegin(FIRSTADDR);
        while (1) {
            if (token.kind == Id) {
                setIdKind(parId);
                enterTpar(token.u.id);
                token = nextToken();
            } else {
                break;
            }
            if (token.kind != Comma) {
                if (token.kind == Id) {
                    errorInsert(Comma);
                    continue;
                } else {
                    break;
                }
            }
            token = nextToken();
        }
        token = checkGet(token, Rparen);
        endpar();
        if (token.kind == Semicolon) {
            errorDelete();
            token = nextToken();
        }
        block(fIndex);
        token = checkGet(token, Semicolon);
    } else {
        errorMissingId();
    }
}

void statement()
{
    int tIndex;
    KindT k;
    int backP, backP2;

    while (1) {
        switch (token.kind) {
        case Id:
            tIndex = searchT(token.u.id, varId);
            setIdKind(k = kindT(tIndex));
            if (k != varId && k != parId) {
                errorType("var/par");
            }
            token = checkGet(nextToken(), Assign);
            expression();
            genCodeT(sto, tIndex);
            return;
        case If:
            token = nextToken();
            condition();
            token = checkGet(token, Then);
            backP = genCodeV(jpc, 0);
            statement();
            backPatch(backP);
            return;
        case Ret:
            token = nextToken();
            expression();
            genCodeR();
            return;
        case Begin:
            token = nextToken();
            while (1) {
                statement();
                while (1) {
                    if (token.kind == Semicolon) {
                        token = nextToken();
                        break;
                    }
                    if (token.kind == End) {
                        token = nextToken();
                        return;
                    }
                    if (isStBeginKey(token)) {
                        errorInsert(Semicolon);
                        break;
                    }
                    errorDelete();
                    token = nextToken();
                }
            }
        case While:
            token = nextToken();
            backP2 = nextCode();
            condition();
            token = checkGet(token, Do);
            backP = genCodeV(jpc, 0);
            statement();
            genCodeV(jmp, backP2);
            backPatch(backP);
            return;
        case Write:
            token = nextToken();
            expression();
            genCodeO(wrt);
            return;
        case WriteLn:
            token = nextToken();
            genCodeO(wrl);
            return;
        case End:
        case Semicolon:
            return;
        default:
            errorDelete();
            token = nextToken();
            continue;
        }
    }
}

int isStBeginKey(Token t)
{
    switch (t.kind) {
    case If:
    case Begin:
    case Ret:
    case While:
    case Write:
    case WriteLn:
        return 1;
    default:
        return 0;
    }
}

void expression()
{
    KeyId k;
    k = token.kind;
    if (k == Plus || k == Minus) {
        token = nextToken();
        term();
        if (k == Minus) {
            genCodeO(neg);
        }
    } else {
        term();
    }
    k = token.kind;
    while (k == Plus || k == Minus) {
        token = nextToken();
        term();
        if (k == Minus) {
            genCodeO(sub);
        } else {
            genCodeO(add);
        }
        k = token.kind;
    }
}

void term()
{
    KeyId k;
    factor();
    k = token.kind;
    while (k == Mult || k == Div || k == Remainder || k == Pow) {
        token = nextToken();
        factor();
        if (k == Mult) {
            genCodeO(mul);
        } else if (k == Div) {
            genCodeO(div);
        } else if (k == Remainder) {
            genCodeO(remainder);
        } else {
            genCodeO(pow);
        }
        k = token.kind;
    }
}

void factor()
{
    int tIndex, i;
    KindT k;
    if (token.kind == Id) {
        tIndex = searchT(token.u.id, varId);
        k = kindT(tIndex);
        setIdKind(k);
        switch (k) {
        case varId:
        case parId:
            genCodeT(lod, tIndex);
            token = nextToken();
            break;
        case constId:
            genCodeV(lit, val(tIndex));
            token = nextToken();
            break;
        case funcId:
            token = nextToken();
            if (token.kind == Lparen) {
                i = 0;
                token = nextToken();
                if (token.kind != Rparen) {
                    for (;;) {
                        expression();
                        i++;
                        if (token.kind == Comma) {
                            token = nextToken();
                            continue;
                        }
                        token = checkGet(token, Rparen);
                        break;
                    }
                } else {
                    token = nextToken();
                }
                if (pars(tIndex) != i)
                    errorMessage("\\#par");
            } else {
                errorInsert(Lparen);
                errorInsert(Rparen);
            }
            genCodeT(cal, tIndex);
            break;
        default:
            printf("not Id\n");
            break;
        }
    } else if (token.kind == Num) {
        genCodeV(lit, token.u.value);
        token = nextToken();
    } else if (token.kind == Lparen) {
        token = nextToken();
        expression();
        token = checkGet(token, Rparen);
    }
    switch (token.kind) {
    case Id:
    case Num:
    case Lparen:
        errorMissingOp();
        factor();
    default:
        return;
    }
}

void condition()
{
    KeyId k;
    if (token.kind == Odd) {
        token = nextToken();
        expression();
        genCodeO(odd);
    } else {
        expression();
        k = token.kind;
        switch (k) {
        case Equal:
        case Lss:
        case Gtr:
        case NotEq:
        case LssEq:
        case GtrEq:
            break;
        default:
            errorType("rel-op");
            break;
        }
        token = nextToken();
        expression();
        switch (k) {
        case Equal:
            genCodeO(eq);
            break;
        case Lss:
            genCodeO(ls);
            break;
        case Gtr:
            genCodeO(gr);
            break;
        case NotEq:
            genCodeO(neq);
            break;
        case LssEq:
            genCodeO(lseq);
            break;
        case GtrEq:
            genCodeO(greq);
            break;
        default:
            errorType("not conditional\n");
            break;
        }
    }
}
