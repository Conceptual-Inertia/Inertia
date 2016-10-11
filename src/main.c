//
//  main.c
//  Inertia
//
//  Created by Tyler Liu on 2016/10/09.
//  Copyright © 2016 Tyler Liu. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
#define INERTIA_IF 0xD //if reg1 = false, skip to reg2
#define INERTIA_RETURN 0xE //return
#define INERTIA_CALL 0xF // call function


unsigned int regs[ NUM_REGS ];
unsigned int memory [ NUM_MEMS ];

FILE *f;
unsigned int len_program;
unsigned int *program;
unsigned int cons[3];

/* fetch the next word from the program */
unsigned int fetch(unsigned int *pc) {
    (*pc) ++;
    //printf("fetch %d\n", *pc - 1);
    return program[*pc - 1];
}

/* instruction fields */
unsigned int instrNum = 0;
unsigned int reg1     = 0;//-1 as const, 0-3 as register, >=4 as memory
unsigned int reg2     = 0;
unsigned int reg3     = 0;

/* fetch and decode a word */
void decode(unsigned int *pc)  {
    
    unsigned int instr = fetch(pc);
    instrNum = instr >> 28;
    //printf("%u", instr >> 28);
    
    reg1 = (instr >> 26) & 3;//0 as memory, 1 as register, 2 as const
    reg2 = (instr >> 24) & 3;
    reg3 = (instr >> 22) & 3;
    if (reg1 == 0) reg1 = 4;
    if (reg2 == 0) reg2 = 4;
    if (reg3 == 0) reg3 = 4;
    if (reg1 == 2) reg1 = ~0;
    if (reg2 == 2) reg2 = ~0;
    if (reg3 == 2) reg3 = ~0;
    
    
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
    
    if (reg1 == ~0){
        cons[0] = fetch(pc);
    }
    
    if (reg2 == ~0){
        cons[1] = fetch(pc);
    }
    
    if (reg3 == ~0){
        cons[2] = fetch(pc);
    }
}

//get memory address
unsigned int* get_add(int i ){
    switch(i){
        case 1:
            if (reg1 == ~0) return &cons[0];
            if (reg1 < 4 ) return &regs[reg1];
            if (reg1 >= 4 && reg1 != ~0) return &memory[reg1 - 4];
            break;
        case 2:
            if (reg2 == ~0) return &cons[1];
            if (reg2 < 4) return &regs[reg2];
            if (reg2 >= 4 && reg2 != ~0) return &memory[reg2 - 4];
            break;
        case 3:
            if (reg3 == ~0) return &cons[2];
            if (reg1 < 4) return &regs[reg3];
            if (reg3 >= 4 && reg2 != ~0) return &memory[reg3 - 4];
            break;
    }
    return 0;
}

/* evaluate the last decoded instruction */
void eval(int *running, unsigned int *pc)
{
    unsigned int pc1;
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
            if (( *get_add(1) ) == 0) *pc = ( *get_add(2) );
            break;
        case INERTIA_RETURN:
            //printf("returning");
            *running = 0;
            break;
        case INERTIA_CALL:
            //run(*get_add(1))
            
            pc1 = *get_add(1) ;
            int running = 1;
            while( running )
            {
                decode( &pc1 );
                eval( &running , &pc1 );
            }

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

// run with program counter
void run()
{
    unsigned int pc = 0;
    int running = 1;
    while( running )
    {
        decode( &pc );
        eval( &running , &pc );
    }
}

//read the char and convert to unsigned
unsigned int fgetu(){
    return (fgetc(f) << 24) +(fgetc(f) << 16) + (fgetc(f) << 8) + (fgetc(f));
}

int main( int argc, const char * argv[] )
{
    
    if (argc == 1) printf("Please enter file name");
    
    //read in file
    
    f = fopen(argv[1], "r");
    len_program = fgetu();
    program = (unsigned *)malloc(len_program * sizeof(unsigned));
    if(!program){
        printf("Fail to allocate instruction array\n");
        exit(1);
    }
    
    for (int i = 0; i < len_program; i ++){
        program[i] = fgetu();
    }
    
    fclose(f);
    
    //execute
    run();
    free(program);
    return 0;
}
