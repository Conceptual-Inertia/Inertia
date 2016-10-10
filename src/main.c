#include <stdio.h>

#ifndef NUM_REGS
  #define NUM_REGS 4
#endif

#ifnedf NUM_MEMS
  #define NUM_MEMS 100
#endif

#define INERTIA_ADD 0x0 // Integer Addition
#define INERTIA_DIV 0x1 // Integer Division
#define INERTIA_MUL 0x2 // Integer Multiplication

#define INERTIA_ILT 0x3 // Integer Less Than
#define INERTIA_IEQ 0x4 // Integer Equal To
#define INERTIA_AND 0x5 // Bitwise AND
#define INERTIA_NOT 0x6 // Bitwise NOT
#define INERTIA_OR 0x7 //  Bitwise OR
#define INERTIA_SHIFTL 0x8 //Bitwise SHIFTL
#define INERTIA_SHIFTR 0x9 //Bitwise SHIFTR

#define INERTIA_PROCEDURE 0xA // Initialize Concept Procedure OUTPUT
#define INERTIA_PRINT 0xB // Print to stdout
#define INERTIA_LOAD 0xC // Load value
#define INERTIA_BRANCH 0xD //branch / goto
#define INERTIA_IF 0xE //branch / if
#define INERTIA_RETURN 0xF //return


unsigned register regs[ NUM_REGS ];

unsigned program[] = { 0x1064, 0x11C8, 0x2201, 0x0000 };

/* program counter */
int pc = 0;

/* fetch the next word from the program */
int fetch() {
    return program[pc++];
}

/* instruction fields */
int instrNum = 0;
int reg1     = 0;
int reg2     = 0;
int reg3     = 0;
int imm      = 0;

/* decode a word */
void decode( int instr )  {
    instrNum = (instr & 0xF000) >> 12;
    reg1     = (instr & 0xF00 ) >>  8;
    reg2     = (instr & 0xF0  ) >>  4;
    reg3     = (instr & 0xF   );
    imm      = (instr & 0xFF  );
}

/* the VM runs until this flag becomes 0 */
int running = 1;

/* evaluate the last decoded instruction */
void eval()
{
    switch( instrNum )
    {
        case 0:
            /* halt */
            printf( "halt\n" );
            running = 0;
            break;
        case 1:
            /* loadi */
            printf( "loadi r%d #%d\n", reg1, imm );
            regs[ reg1 ] = imm;
            break;
        case 2:
            /* add */
            printf( "add r%d r%d r%d\n", reg1, reg2, reg3 );
            regs[ reg1 ] = regs[ reg2 ] + regs[ reg3 ];
            break;
    }
}

/* display all registers as 4-digit hexadecimal words */
void showRegs()
{
    int i;
    printf( "regs = " );
    for( i=0; i<NUM_REGS; i++ )
        printf( "%04X ", regs[ i ] );
    printf( "\n" );
}

void run()
{
    while( running )
    {
        showRegs();
        int instr = fetch();
        decode( instr );
        eval();
    }
    showRegs();
}

int main( int argc, const char * argv[] )
{
    run();
    return 0;
}
