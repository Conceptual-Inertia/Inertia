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

#define INERTIA_ADD 0x0 // Integer Addition
#define INERTIA_DIV 0x1 // Integer Division
#define INERTIA_MUL 0x2 // Integer Multiplication

#define INERTIA_LTN 0x3 // Integer Less Than
#define INERTIA_EQL 0x4 // Integer Equal To
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


unsigned regs[ NUM_REGS ];
unsigned memory [ NUM_MEMS ];
unsigned long program[] = { 0x1064, 0x11C8, 0x2201, 0x0000 };
unsigned cons[3];

/* program counter */
int pc = 0;

/* fetch the next word from the program */
long fetch() {
    return program[pc++];
}

/* instruction fields */
int instrNum = 0;
int reg1     = 0;//-1 as const, 0-3 as register, >=4 as memory
int reg2     = 0;
int reg3     = 0;
int imm      = 0;

/* decode a word */
void decode( long instr )  {
    
    instrNum = instr >> 60;
    reg1 = (instr >> 58) & 3;//0 as memory, 1 as register, 2 as const
    reg2 = (instr >> 56) & 3;
    reg3 = (instr >> 54) & 3;
    if (reg1 == 0) reg1 = 4;
    if (reg2 == 0) reg2 = 4;
    if (reg3 == 0) reg3 = 4;
    if (reg1 == 2) reg1 = -1;
    if (reg2 == 2) reg2 = -1;
    if (reg3 == 2) reg3 = -1;
    
    if (reg1 == 1) {
        reg1 = (instr >> 52) & 3;
    }
    
    if (reg2 == 1) {
        reg2 = (instr >> 50) & 3;
    }
    
    if (reg3 == 1) {
        reg3 = (instr >> 48) & 3;
    }
    
    if (reg1 == 4){
        reg1 = ((instr >> 32) & 65535) + 4;
    }
    
    if (reg2 == 4){
        reg2 = ((instr >> 16) & 65535) + 4;
    }
    
    if (reg3 == 4){
        reg3 = ((instr >> 0) & 65535) + 4;
    }
    
    if (reg1 == -1){
        cons[0] = (int) fetch();
    }
    
    if (reg2 == -1){
        cons[1] = (int) fetch();
    }
    
    if (reg3 == -1){
        cons[2] = (int) fetch();
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

/* the VM runs until this flag becomes 0 */
int running = 1;

/* evaluate the last decoded instruction */
void eval()
{
    switch( instrNum )
    {
        
        case INERTIA_ADD:
            /* add */
            *get_add(reg1) = *get_add(reg2) + *get_add(reg3);
            break;
        case INERTIA_DIV:
            /* divide */
            *get_add(reg1) = *get_add(reg2) / *get_add(reg3);
            break;
        case INERTIA_MUL:
            /* multiply */
            *get_add(reg1) = *get_add(reg2) * *get_add(reg3);
            break;
            
        case INERTIA_LTN :
            //Less Than
            if (*get_add(reg2) < *get_add(reg3)) *get_add(reg1) = ~0;
            else *get_add(reg1) = 0;
            break;
        case INERTIA_EQL :
            //Less Than
            if (*get_add(reg2) == *get_add(reg3)) *get_add(reg1) = ~0;
            else *get_add(reg1) = 0;
            break;
        case INERTIA_AND:
            *get_add(reg1) = *get_add(reg2) & *get_add(reg3);
            break;
        case INERTIA_OR:
            *get_add(reg1) = *get_add(reg2) | *get_add(reg3);
            break;
        case INERTIA_NOT:
            *get_add(reg1) = ~*get_add(reg2);
            break;
        case INERTIA_SHIFTL:
            *get_add(reg1) = *get_add(reg2) << *get_add(reg3);
            break;
        case INERTIA_SHIFTR:
            *get_add(reg1) = *get_add(reg2) >> *get_add(reg3);
            break;
            
        case INERTIA_LOAD:
            *get_add(reg1) = *get_add(reg2);
            break;
        case INERTIA_PRINT:
            printf("%d\n", *get_add(reg1));
        
        case INERTIA_BRANCH:
            /* halt if -1 */
            //todo others
            if (*get_add(reg1) == 0) running = 0;
            break;
    }
}

/* display all registers as 8-digit hexadecimal words */
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
