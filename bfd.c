// bfd - bf disassembler. There is NO LICENSE.

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int  p = 0;
char data[64*1024], prog[64*1024];

// Variables for printing
const char lowVars[] = "ABCDEFGHIJKLMN";
int nIndent = 0;
char indentBuf[60];

// Buffer output to prevent it from getting
// jumbled with the extensive debug output.
char outputBuf[80];
int  outbufIndex;

#define SPACE ' '

char *indent() {
    int n = 2*nIndent;
    char *end = (n >= sizeof(indentBuf)-1) ? indentBuf+sizeof(indentBuf)-1 : indentBuf+n;
    char *p = indentBuf;
    while (p < end) {
        *p++ = SPACE;
    }
    *p = 0;
    return indentBuf;
}

void printIncDec(int p, int ct) {
    if (p < sizeof(lowVars)) {
        printf("%s%c %s %d // (%c == %d)\n", indent(),
            lowVars[p], (ct >= 0) ? "+=" : "-=", abs(ct), lowVars[p], data[p]);
    } else {
        printf("%sdata[%d] %s %d // (data[%d] = %d)\n", indent(),
            p, (ct >= 0) ? "+=" : "-=", abs(ct), p, data[p]);
    }
}

void interpret(char *instrPtr)
{
    int incCount = 0;
    printf("%s{\n", indent());
    nIndent++;

    while( *instrPtr ) {
        switch(*instrPtr++) {
        case '<':
            if (incCount != 0) {
                printIncDec(p, incCount);
                incCount = 0;
            }
            p--;
            break;
        case '>':
            if (incCount != 0) {
                printIncDec(p, incCount);
                incCount = 0;
            }
            p++;
            break;
        case '+':
            incCount++;
            data[p]++;
            break;
        case '-':
            incCount--;
            data[p]--;
            break;
        case '.':
            if (incCount != 0) {
                printIncDec(p, incCount);
                incCount = 0;
            }
            if (data[p] == '\n') {
                printf("%sPUT(%s)\n", indent(), "\\n");
            } else {
                printf("%sPUT(%c)\n", indent(), data[p]);
            }
            outputBuf[outbufIndex++] = data[p];
            if (outbufIndex >= sizeof(outputBuf) || data[p] == '\n') {
                printf("[OUTPUT] %s", outputBuf);
                fflush(stdout);
                outbufIndex = 0;
                memset(outputBuf, 0, sizeof(outputBuf));
            }
            break;
        case ',':
            if (incCount != 0) {
                printIncDec(p, incCount);
                incCount = 0;
            }
            // If there's a buffered prompt,
            // flush it before we get a char.
            if (outbufIndex != 0) {
                printf("[OUTPUT] %s", outputBuf);
                fflush(stdout);
                outbufIndex = 0;
                memset(outputBuf, 0, sizeof(outputBuf));
            }
            data[p]=getchar();
            if (p < sizeof(lowVars)) {
                printf("%s%c = GET('%c')\n", indent(), lowVars[p], data[p]);
            } else {
                printf("%sdata[%d] = GET('%c')\n", indent(), p, data[p]);
            }
            fflush(stdout);
            break;
        case '[':
        {
            if (incCount != 0) {
                printIncDec(p, incCount);
                incCount = 0;
            }
            if (p < sizeof(lowVars)) {
                printf("%swhile(%c != 0) // %c == %d\n", indent(), lowVars[p], lowVars[p], data[p]);
            } else {
                printf("%swhile(data[%d] != 0) { // data[%d] == %d\n", indent(), p, p, data[p]);
            }

            // Depth is one because we just saw a "start of loop". Worst case,
            // loop will terminate because the program loader put a 0 after the
            // last instruction it loaded.
            char depth = 1;
            char *startOfLoop = instrPtr;
            while (depth != 0 && *instrPtr) {
                depth += *instrPtr=='[';
                depth -= *instrPtr==']';
                instrPtr++;
            }
            if (depth == 0) {
                instrPtr[-1]=0;
                while( data[p] ) {
                    interpret(startOfLoop);
                }
                instrPtr[-1]=']';
                break;
            }
            // else fall through
        }
        case ']':
            printf("unbalanced bracket near %ld\n", (instrPtr - prog));
            exit(0);
        }
    }
    if (incCount != 0) {
        printIncDec(p, incCount);
        incCount = 0;
    }
    nIndent--;
    printf("%s}\n", indent());
}

int main(int ac, char *av[])
{
    if (ac != 2) {
        printf("Usage: %s bfi-file\n", av[0]);
        exit(1);
    }

    FILE *fp = fopen(av[1],"r");
    if (!fp) {
        printf("Open %s failed (\"%s\")\n", av[1], strerror(errno));
        exit(1);
    }

    // Load the program from the file with a terminating nul char
    // (binary 0). The comments are loaded along with the code,
    // then ignored by the interpreter. The "-1" on the length
    // check accommodates the terminating nul.

    int nOp = 0;
    char b;
    while( (b=getc(fp))>0 ) {
        if (nOp >= sizeof(prog) - 1) {
            printf("Program is too long.");
            exit(2);
        }
        prog[nOp++] = b;
    }
    prog[nOp++] = 0;

    interpret(prog);
}

