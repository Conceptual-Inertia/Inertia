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


uint32_t regs[ NUM_REGS ];
uint32_t memory [ NUM_MEMS ];

FILE *f;
uint32_t len_program;
uint32_t *program;
uint32_t cons[3];

/* fetch the next word from the program */
uint32_t fetch(uint32_t *pc) {
    (*pc) ++;
    //printf("fetch %d\n", *pc - 1);
    return program[*pc - 1];
}

/* instruction fields */
static uint32_t instrNum = 0;
static uint32_t reg1     = 0;//-1 as const, 0-3 as register, >=4 as memory
static uint32_t reg2     = 0;
static uint32_t reg3     = 0;

/* fetch and decode a word */
void decode(uint32_t *pc)  {
    
    uint32_t instr = fetch(pc);
    uint32_t instr2 = fetch(pc);
    instrNum = instr >> 28;
    //printf("%u", instr >> 28);
    
    //0 as memory, 1 as register, 2 as const
    
    switch ((instr >> 26) & 3) {//reg1
        case 0:
            reg1 = (instr & 65535) + 4;
            break;
        case 1:
            reg1 = (instr >> 20) & 3;
            break;
        case 2:
            cons[0] = fetch(pc);
            break;
    }
    
    switch ((instr >> 24) & 3) {//reg2
        case 0:
            reg2 = ((instr2 >> 16) & 65535) + 4;
            break;
        case 1:
            reg2 = (instr >> 18) & 3;
            break;
        case 2:
            cons[1] = fetch(pc);
            break;
    }
    
    switch ((instr >> 22) & 3) {//reg3
        case 0:
            reg3 = (instr2 & 65535) + 4;
            break;
        case 1:
            reg3 = (instr >> 16) & 3;
            break;
        case 2:
            cons[2] = fetch(pc);
            break;
    }
    
}

//get memory address
uint32_t* get_add(int i ){
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
void eval(int *running, uint32_t *pc)
{
    uint32_t pc1;
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
            if (( *get_add(2) ) < ( *get_add(3) )) ( *get_add(1) ) = (uint32_t)~0;
            else ( *get_add(1) ) = 0;
            break;
        case INERTIA_EQL :
            //Equal to
            if (( *get_add(2) ) == ( *get_add(3) )) ( *get_add(1) ) = (uint32_t)~0;
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
            int running1 = 1;
            while( running1 )
            {
                decode( &pc1 );
                eval( &running1 , &pc1 );
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
    uint32_t pc = 0;
    int running = 1;
    while( running )
    {
        decode( &pc );
        eval( &running , &pc );
    }
}

int main( int argc, const char * argv[] )
{
    
    if (argc == 1) printf("Please enter file name");
    
    //read in file
    
    f = fopen(argv[1], "r");
    len_program = (uint32_t)(fgetc(f) << 24) +(fgetc(f) << 16) + (fgetc(f) << 8) + (fgetc(f));
    program = (uint32_t *)malloc(len_program * sizeof(unsigned));
    if(!program){
        printf("Failed to allocate instruction array\n");
        exit(1);
    }
    
    for (int i = 0; i < len_program; i ++){
        program[i] = (uint32_t)(fgetc(f) << 24) +(fgetc(f) << 16) + (fgetc(f) << 8) + (fgetc(f));
    }
    
    fclose(f);
    
    //execute
    run();
    free(program);
    return 0;
}
