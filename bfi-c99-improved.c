// This is an improved version of the C99-compatible version of the
// original BF program. The primary improvement is changing all the
// single-letter variable names to improve their readability. I have
// also added timing for the execution and improved the error messages
// a little. There are NO semantic changes at all so far as I know.
//
// The original BF program, dated 1993-06-09, was downloaded from:
// https://aminet.net/package.php?package=dev/lang/brainfuck-2.lha#contents
// The download is an "lha" style archive which was expanded with "lha"
// (brew install lha) on macos X (Sonoma) in March 2024.
//
// There is NO license on the download. It's been downloaded over 18000
// times, however. I'm treating it as though it's "public domain". This
// means I am not applying a license.

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int  p;
int printStats; // bool
char data[5000], prog[5000];
uint64_t nExecuted = 0;
uint64_t nCommentsExecuted = 0;

void interpret(char *instrPtr)
{
	while( *instrPtr ) {
		//if(strchr("<>+-,.[]\n",*instrPtr))printf("%c",*instrPtr);
        nExecuted++;
		switch(*instrPtr++) {
		case '<': p--;        break;
		case '>': p++;        break;
		case '+': data[p]++;     break;
		case '-': data[p]--;     break;
		case '.': putchar(data[p]); fflush(stdout); break;
		case ',': data[p]=getchar();fflush(stdout); break;
		case '[':
        {
            // depth is one because we just saw a "start of loop".
            // Worst case, the while loop will terminate because
            // the program loader put a 0 after the last
            // instruction it loaded.
            char depth = 1;
            char *startOfLoop = instrPtr;
            while (depth != 0 && *instrPtr) {
				depth += *instrPtr=='[';
                depth -= *instrPtr==']';
                instrPtr++;
            }
			if (depth == 0) {
				instrPtr[-1]=0;
				while( data[p] )
					interpret(startOfLoop);
				instrPtr[-1]=']';
				break;
			}
            // else fall through
        }
		case ']':
            printf("unbalanced bracket near %ld\n", (instrPtr - prog));
            exit(0);
		case '#':
			if (printStats)
				printf("%2d %2d %2d %2d %2d %2d %2d %2d %2d %2d\n%*s\n",
                   data[0], data[1], data[2], data[3], data[4], data[5],
                   data[6], data[7], data[8], data[9], 3*p+2,"^");
			break;
        default:
            nCommentsExecuted++;
            break;
		}
        // I don't really understand this: the original programmer (Mueller)
        // allocated 5000 data bytes in the array, but the program can really
        // really only use 101 of them, apparently to prevent bugs.
		if( p<0 || p>100)
			puts("RANGE ERROR\n"), exit(0);
	}
}

int main(int ac, char *av[])
{
    if (ac < 2 || ac > 3) {
        printf("Usage: %s bfi-file [any-arg-to-show-dump-on-#]\n", av[0]);
        exit(1);
    }
	printStats = (ac == 3);

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
    if (printStats) {
        printf("Running (%d operation bytes)...\n", nOp);
    }

    int timing = 0;
    struct timespec start, end;
    if (printStats && clock_gettime(CLOCK_MONOTONIC, &start) == 0) {
        timing = 1;
    }

    interpret(prog);

    if (timing && clock_gettime(CLOCK_MONOTONIC, &end) == 0) {
        uint64_t million = (uint64_t)1000*1000;
        uint64_t billion = (uint64_t)1000*million;

        uint64_t runtimeNanos = billion * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
        long secs = runtimeNanos / billion;
        long millis = (runtimeNanos - (secs * billion)) / million;
        printf("%ld.%-3ld %-s\n", secs, millis, "seconds elapsed");

        double nanosPerOp = (double)runtimeNanos / (double)nExecuted;
        printf("%lld comment bytes executed, %lld operations executed (about %.1f ns/operation)\n",
               nCommentsExecuted, nExecuted, nanosPerOp);
    }
}

