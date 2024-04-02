// This is the original BF program with minimal changes required
// to compile cleanly on a modern C compiler. The compiler used is:
// Apple clang version 15.0.0 (clang-1500.3.9.4)
//
// The original BF program, dated 1993-06-09, was downloaded from:
// https://aminet.net/package.php?package=dev/lang/brainfuck-2.lha#contents
// The download is an "lha" style archive which was expanded with "lha"
// (brew install lha) on macos X (Sonoma) in March 2024.
//
// There is NO license on the download. It's been downloaded over 18000
// times, however. I'm treating it as though it's "public domain". I am
// not applying a license.

#include <stdlib.h>
#include <stdio.h>

int  p, r, q;
char a[5000], f[5000], b, o, *s=f;

void interpret(char *c)
{
	char *d;

	r++;
	while( *c ) {
		//if(strchr("<>+-,.[]\n",*c))printf("%c",*c);
		switch(o=1,*c++) {
		case '<': p--;        break;
		case '>': p++;        break;
		case '+': a[p]++;     break;
		case '-': a[p]--;     break;
		case '.': putchar(a[p]); fflush(stdout); break;
		case ',': a[p]=getchar();fflush(stdout); break;
		case '[':
			for( b=1,d=c; b && *c; c++ )
				b+=*c=='[', b-=*c==']';
			if(!b) {
				c[-1]=0;
				while( a[p] )
					interpret(d);
				c[-1]=']';
				break;
			}
		case ']':
			puts("UNBALANCED BRACKETS"), exit(0);
		case '#':
			if(q>2)
				printf("%2d %2d %2d %2d %2d %2d %2d %2d %2d %2d\n%*s\n",
				       *a,a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9],3*p+2,"^");
			break;
		default: o=0;
		}
		if( p<0 || p>100)
			puts("RANGE ERROR"), exit(0);
	}
	r--;
#if ANCIENT_AMIGA_CTRL_C_HANDLING
	chkabort();
#endif
}

int main(int argc,char *argv[])
{
	FILE *z;

	q=argc;

	z = fopen(argv[1],"r");
	if (z) {
		while( (b=getc(z))>0 )
			*s++=b;
		*s=0;
		interpret(f);
	}
}

