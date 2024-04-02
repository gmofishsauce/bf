# BF

This is a copy of the Brainfuck repo created by Urban Mueller in 1993. BF is a simple language, similar to a Turing machine but much easier to program. Yet programming in BF is much more difficult than any real assembly language. The details are described below.

The file `bfi.c` is the original BF interpreter, while `bfc.asm` is the original compiler. The compiler is written in Amiga assembly language so it's of historical interest only. The interpreter contains an Amiga-specific function call and does not compile under C99. There is also some BF source code in `src/`, including a program that finds prime numbers up to a given value.

The original archive (LHA file) is checked in to the repo. It contains Amiga binaries for the compiler and interpreter that I did not bother checking in separately. I have not attempted any work on the compiler either.

Instead, I have worked on the interpreter, `bfi.c`. I started by making the minimal set of changes required to get a clean compile under C99 (specifically, `Apple clang version 15.0.0 (clang-1500.3.9.4)`). This is `bfi-c99.c`. I then made a modest set of improvements. This is `bfi-c99-improved.c`. The improvements include renaming all the one-letter variable names, removing some unused variables, improving error messages, checking for overlength programs, and adding some execution timing statistics.

At this point I committed the code to this repo on Github.

## LICENSE

This code has NO LICENSE. But it was apparently placed in a downloadable archive by author, which in my view constitutes publication, and it's been downloaded over 18,000 times. I am treating it like public domain code. The public domain means both more and less than many people believe, and it's not recognized in all jurisdictions (remember: on the internet, your nation's laws are just a *local* ordinance).

If you're out there, Urban Mueller, and you want me to take this down - my email name is six letters: the IATA airport code for Portland, Oregon, USA, followed by my initials, "jjb". At the leading search engine's email service.

# THE ORIGINAL [README](https://aminet.net/package.php?package=dev/lang/brainfuck-2.lha#contents):

This archive contains the following programs:

- bfc.asm      Source for the compiler
- bfi.c        Source for the interpreter (portable)
- src/         Some example programs in 'brainfuck'
- src/atoi.b   Reads a number from stdin
- src/div10.b  Divides the number under the pointer by 10
- src/hello.b  The ubiquitous "Hello World!"
- src/mul10.b  Multiplies the number under the pointer by 10
- src/prime.b  Computes the primes up the a variable limit
- src/varia.b  Small program elements like SWAP, DUP etc.
- brainfuck-2.lha: the original archive.

2024 note: the original archive contains two Amiga binaries which I did not check in separately.

## WHAT'S NEW

Yes, I squeezed another ridiculous 56 bytes out of the compiler. They have
their price, though: The new compiler requires OS 2.0, operates on a byte 
array instead of longwords, and generates executables that are always 64K 
in size. Apart from that the language hasn't changed. Again: OS 2.0 *required* for the compiler and the compiled programs.
The interpreter works fine under any OS version. And yes, thanks to Chris
Schneider for his ideas how to make the compiler shorter.


## THE LANGUAGE

The language 'brainfuck' knows the following commands:

```
 Cmd  Effect                                 Equivalent in C
 ---  ------                                 ---------------
 +    Increases element under pointer        array[p]++;
 -    Decrases element under pointer         array[p]--;
 >    Increases pointer                      p++;
 <    Decreases pointer                      p--;
 [    Starts loop, counter under pointer     while(array[p]) {
 ]    Indicates end of loop                  }
 .    Outputs ASCII code under pointer       putchar(array[p]);
 ,    Reads char and stores ASCII under ptr  array[p]=getchar();
```

All other characters are ignored. The 30000 array elements and p are being
initialized to zero at the beginning.  Now while this seems to be a pretty
useless language, it can be proven that it can compute every solvable
mathematical problem (if we ignore the array size limit and the executable
size limit).


## THE COMPILER

The compiler does not check for balanced brackets; they better be. It reads the source from stdin and writes the executable to stdout. Note that the executable is always 65536 bytes long, and usually won't be executable if brackets aren't balanced. OS 2.0 required for the compiler and the compiled program.

2024 note: the compiler, `bfc.asm`, is written in Amiga assembly language
so it's of historical interest only.


## THE INTERPRETER

For debugging, there's also an interpreter. It expects the name of the 
program to  interpret on the command line and accepts an additional command: Whenever a '#' is met in the source and a second argument was passwd to the interpreter, the first few elements of the array are written to stdout as numbers.

2024 note: the original interpreter, `bfi.c`, does not compile under C99. See the notes at the top of this file.

Enjoy

Urban Mueller     (2024 note: email address elided)

