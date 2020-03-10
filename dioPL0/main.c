// The main routine of all

#include "getSource.h"
#include <stdio.h>

int main(void)
{
    char fileName[30];
    printf("Enter source file name\n");
    scanf("%s", fileName);
    if (!openSource(fileName)) {
        return 0;
    }
    if (compile()) {
        execute();
    }
    closeSource();
}
