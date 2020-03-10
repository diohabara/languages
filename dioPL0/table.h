typedef enum kindT {
    varId,
    funcId,
    parId,
    constId
} KindT;
typedef struct relAddr {
    int level;
    int addr;
} RelAddr;

void blockBegin(int firstAddr);
void blockEnd();
int bLevel();
int fPars();
int enterTfunc(char* id, int v);
int enterTvar(char* id);
int enterTpar(char* id);
int enterTconst(char* id, int v);
void endpar();
void changeV(int ti, int newVal);

int searchT(char* id, KindT k);
KindT kindT(int i);

RelAddr relAddr(int ti);
int val(int ti);
int pars(int ti);
int frameL();
