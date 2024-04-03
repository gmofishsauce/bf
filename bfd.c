// bfd - bf disassembler. There is NO LICENSE.

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// BF state
int  p = 0;
char data[64*1024];
char prog[64*1024];

// Variables for printing. The first 14 locations in the array data[] are named
// A, B, C, etc. The rest of the locations are named data[n].
const char lowVars[] = "ABCDEFGHIJKLMN";
int incDecCount = 0;
int nIndent = 0;
char indentBuf[60];

// Buffer output to prevent it from getting jumbled with the extensive debug output.
// Output is issued when a newline is seen or a , (comma) operator ("getchar") is
// encountered (because the buffered output might be a prompt). Output is marked
// with an identifying string, "[OUTPUT]", to support grep.
char outputBuf[80];
int  outbufIndex;

#define SPACE ' '

// Print the proper indent's worth of spaces by copying up to "max" spaces
// into the spaces buffer, nul terminating the buffer, and returning it.
// The "-1" on the end variable leaves space for the terminating nul.
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

// If the accumulated count of increments and decrements is not 0,
// print it with the proper indent and clear the count.
void printIncDec() {
    if (incDecCount == 0) {
        return;
    }
    if (p < sizeof(lowVars)) {
        printf("%s%c %s %d // (%c == %d)\n", indent(),
            lowVars[p], (incDecCount >= 0) ? "+=" : "-=", abs(incDecCount), lowVars[p], data[p]);
    } else {
        printf("%sdata[%d] %s %d // (data[%d] = %d)\n", indent(),
            p, (incDecCount >= 0) ? "+=" : "-=", abs(incDecCount), p, data[p]);
    }
    incDecCount = 0;
}

// It's difficult to find the output from the BF program buried in the massive
// trace output. To help, we buffer output until either a newline or GET (comma)
// is seen and then push the output preceded by string "[OUTPUT]". This helps
// with grep.
void flush() {
    if (outbufIndex > 0) {
        printf("[OUTPUT] %s", outputBuf);
        fflush(stdout);
        outbufIndex = 0;
        memset(outputBuf, 0, sizeof(outputBuf));
    }
}

void interpret(char *instrPtr)
{
    printf("%s{\n", indent());
    nIndent++;

    while( *instrPtr ) {
        switch(*instrPtr++) {
        case '<':
            printIncDec();
            p--;
            break;
        case '>':
            printIncDec();
            p++;
            break;
        case '+':
            incDecCount++;
            data[p]++;
            break;
        case '-':
            incDecCount--;
            data[p]--;
            break;
        case '.':
            printIncDec();
            if (data[p] == '\n') {
                printf("%sPUT(%s)\n", indent(), "\\n");
            } else {
                printf("%sPUT(%c)\n", indent(), data[p]);
            }
            // We buffer output to make it easier to find
            outputBuf[outbufIndex++] = data[p];
            if (outbufIndex >= sizeof(outputBuf) || data[p] == '\n') {
                flush();
            }
            break;
        case ',':
            printIncDec();
            // If there's a buffered prompt,
            // flush it before we get a char.
            if (outbufIndex != 0) {
                flush();
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
            printIncDec();
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
        case -2:
            if (p < sizeof(lowVars)) {
                printf("%sclear %c // %c == 0\n", indent(), lowVars[p], lowVars[p]);
            } else {
                printf("%sclear data[%d] // data[%d] == 0)\n", indent(), p, p);
            }
            data[p] = 0;
            break;
        case -1:
            break;
        default:
            if (instrPtr[-1] >= 0) {
                // only print ASCII chars
                putchar(*(instrPtr-1));        
            }
            break;
        }
    }
    printIncDec();
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
    // check accommodates the terminating nul. Note that trying
    // to load a binary file having bytes >= 0x80 unsigned will
    // not work; the first such byte is treated as end of file.
    // The proof of concept condenser takes advantage of this by
    // using bytes between unsigned 0x80 and 0xFF as compressed
    // opcodes which are understood by the interpreter.

    int nOp = 0;
    char b;
    while( (b=getc(fp))>0 ) {
        if (nOp >= sizeof(prog) - 1) {
            printf("Program is too long.");
            exit(2);
        }
        // This is a proof of concept condenser for the sequence [-]
        // This is equivalent to "set the byte at the pointer to 0".
        if (b == ']' && nOp >= 2 && prog[nOp-1] == '-' && prog[nOp-2] == '[') {
            prog[nOp-2] = 0xFE; // "CLEAR data[p]"
            prog[nOp-1] = 0xFF; // noop
            prog[nOp++] = 0xFF; // noop
        } else {
            prog[nOp++] = b;
        }
    }
    prog[nOp++] = 0;
    // We could implement SMBF (self-modifying brainfuck) by setting the data
    // pointer to nOp here and having just one array to hold code and data. TBD.
    interpret(prog);
}

