#include <stdio.h>

#define NUM_REGS 4


unsigned regs[ NUM_REGS ];

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