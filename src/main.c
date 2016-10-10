//
//  main.c
//  Inertia
//
//  Created by Tyler Liu on 2016/10/09.
//  Copyright © 2016 Tyler Liu. All rights reserved.
//

#include <stdio.h>

#define NUM_REGS 4

#define NUM_MEMS 65536

#define INERTIA_ADD 0x0 // Addition
#define INERTIA_DIV 0x1 // Division
#define INERTIA_MUL 0x2 // Multiplication

#define INERTIA_LTN 0x3 // Less Than
#define INERTIA_EQL 0x4 // Equal To
#define INERTIA_AND 0x5 // Bitwise AND
#define INERTIA_NOT 0x6 // Bitwise NOT
#define INERTIA_OR 0x7 //  Bitwise OR
#define INERTIA_SHIFTL 0x8 //Bitwise SHIFTL
#define INERTIA_SHIFTR 0x9 //Bitwise SHIFTR

#define INERTIA_PRINT 0xA // Print to stdout
#define INERTIA_LOAD 0xB // Load value
#define INERTIA_GOTO 0xC //goto
#define INERTIA_IF 0xD //if
#define INERTIA_RETURN 0xE //return


unsigned regs[ NUM_REGS ];
unsigned memory [ NUM_MEMS ];
unsigned program[] = { 0x66800000, 0x00000000, 0x00000007, 0x00000008, 0xA4000000, 0x00000000, 0xE0000000, 0x00000000 };
unsigned cons[3];

/* fetch the next word from the program */
unsigned fetch(int *pc) {
    (*pc) ++;
    printf("fetch %d\n", *pc - 1);
    return program[*pc - 1];
}

/* instruction fields */
unsigned instrNum = 0;
int reg1     = 0;//-1 as const, 0-3 as register, >=4 as memory
int reg2     = 0;
int reg3     = 0;
int imm      = 0;

/* fetch and decode a word */
void decode(int *pc)  {
    
    unsigned instr = fetch(pc);
    instrNum = instr >> 28;
    //printf("%u", instr >> 28);

    reg1 = (instr >> 26) & 3;//0 as memory, 1 as register, 2 as const
    reg2 = (instr >> 24) & 3;
    reg3 = (instr >> 22) & 3;
    if (reg1 == 0) reg1 = 4;
    if (reg2 == 0) reg2 = 4;
    if (reg3 == 0) reg3 = 4;
    if (reg1 == 2) reg1 = -1;
    if (reg2 == 2) reg2 = -1;
    if (reg3 == 2) reg3 = -1;
    
    
    if (reg1 == 1) {
        reg1 = (instr >> 20) & 3;
    }
    
    if (reg2 == 1) {
        reg2 = (instr >> 18) & 3;
    }
    
    if (reg3 == 1) {
        reg3 = (instr >> 16) & 3;
    }
    
    if (reg1 == 4){
        reg1 = (instr & 65535) + 4;
    }
    
    instr = fetch(pc);
    if (reg2 == 4){
        reg2 = ((instr >> 16) & 65535) + 4;
    }
    
    if (reg3 == 4){
        reg3 = (instr & 65535) + 4;
    }
    
    if (reg1 == -1){
        cons[0] = fetch(pc);
    }
    
    if (reg2 == -1){
        cons[1] = fetch(pc);
    }
    
    if (reg3 == -1){
        cons[2] = fetch(pc);
    }
}

//get memory address
unsigned int* get_add(int i ){
    switch(i){
        case 1:
            if (reg1 == -1) return &cons[0];
            if (reg1 >= 0 && reg1 < 4) return &regs[reg1];
            if (reg1 >= 4) return &memory[reg1 - 4];
            break;
        case 2:
            if (reg2 == -1) return &cons[1];
            if (reg2 >= 0 && reg2 < 4) return &regs[reg2];
            if (reg2 >= 4) return &memory[reg2 - 4];
            break;
        case 3:
            if (reg3 == -1) return &cons[2];
            if (reg3 >= 0 && reg1 < 4) return &regs[reg3];
            if (reg3 >= 4) return &memory[reg3 - 4];
            break;
    }
    return 0;
}

/* evaluate the last decoded instruction */
void eval(int *running, int *pc)
{
    switch( instrNum )
    {
        
        case INERTIA_ADD:
            /* add */
            ( *get_add(1) ) = ( *get_add(2) ) + ( *get_add(3) );
            break;
        case INERTIA_DIV:
            /* divide */
            ( *get_add(1) ) = ( *get_add(2) ) / ( *get_add(3) );
            break;
        case INERTIA_MUL:
            /* multiply */
            ( *get_add(1) ) = ( *get_add(2) ) * ( *get_add(3) );
            break;
            
        case INERTIA_LTN :
            //Less Than
            if (( *get_add(2) ) < ( *get_add(3) )) ( *get_add(1) ) = ~0;
            else ( *get_add(1) ) = 0;
            break;
        case INERTIA_EQL :
            //Less Than
            if (( *get_add(2) ) == ( *get_add(3) )) ( *get_add(1) ) = ~0;
            else ( *get_add(1) ) = 0;
            break;
        case INERTIA_AND:
            ( *get_add(1) ) = ( *get_add(2) ) & ( *get_add(3) );
            break;
        case INERTIA_OR:
            ( *get_add(1) ) = ( *get_add(2) ) | ( *get_add(3) );
            break;
        case INERTIA_NOT:
            ( *get_add(1) ) = ~( *get_add(2) );
            break;
        case INERTIA_SHIFTL:
            ( *get_add(1) ) = ( *get_add(2) ) << ( *get_add(3) );
            break;
        case INERTIA_SHIFTR:
            ( *get_add(1) ) = ( *get_add(2) ) >> ( *get_add(3) );
            break;
            
        case INERTIA_LOAD:
            ( *get_add(1) ) = ( *get_add(2) );
            break;
        case INERTIA_PRINT:
            printf("%d\n", ( *get_add(1) ));
            break;
	case INERTIA_GOTO:
            *pc = ( *get_add(1) );
            break;
        case INERTIA_IF:
            if (( *get_add(1) ) != 0) *pc = ( *get_add(2) );
            break;
        case INERTIA_RETURN:
            //printf("returning");
            *running = 0;
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

void run(int pc)
{
    int running = 1;
    while( running )
    {
        decode( &pc );
        eval( &running , &pc );
    }
}

int main( int argc, const char * argv[] )
{
    run(0);
    return 0;
}
