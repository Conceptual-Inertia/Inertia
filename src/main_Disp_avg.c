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
#include <time.h>

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
#define INERTIA_INC 0x8 //Increase by 1
#define INERTIA_DEC 0x9 //Decrease by 1

#define INERTIA_PRINT 0xA // Print to stdout
#define INERTIA_LOAD 0xB // Load value
#define INERTIA_GOTO 0xC //goto
#define INERTIA_IF 0xD //if par1 == false, skip to par2
#define INERTIA_RETURN 0xE //return
#define INERTIA_CALL 0xF // call function

//timing constant
int numdisp;
clock_t tdisp;
clock_t tfetch;

typedef struct I_instr_t{
    uint32_t instr;
}I_instr_t;


uint32_t regs[ NUM_REGS ];
uint32_t memory [ NUM_MEMS ];

FILE *f;
uint32_t len_program;
I_instr_t *program;
uint32_t cons[3];

uint32_t fetch(uint32_t *pc) {
    clock_t start = clock(), diff;
    if (*pc >= len_program) {
        printf("Instruction array out of bound\n");
        exit(2);
    }
    (*pc) ++;
    tfetch += clock() - start;
    //printf("fetch %d\n", *pc - 1);
    return program[*pc - 1].instr;
}

/* instruction fields */
static uint32_t instrNum = 0;
static int32_t par1     = 0;//-1 as const, 0-3 as register, >=4 as memory
static int32_t par2     = 0;
static int32_t par3     = 0;

/* fetch and decode a word */
void decode(uint32_t *pc)  {
    
    uint32_t instr = fetch(pc);
    uint32_t instr2 = fetch(pc);
    instrNum = instr >> 28;
    //printf("%u", instr >> 28);
    
    //0 as memory, 1 as register, 2 as const
    
    switch ((instr >> 26) & 3) {//par1
        case 0:
            par1 = (instr & 65535) + 4;
            break;
        case 1:
            par1 = (instr >> 20) & 3;
            break;
        case 2:
            par1 = -1;
            cons[0] = fetch(pc);
            break;
    }
    
    switch ((instr >> 24) & 3) {//par2
        case 0:
            par2 = ((instr2 >> 16) & 65535) + 4;
            break;
        case 1:
            par2 = (instr >> 18) & 3;
            break;
        case 2:
            par2 = -2;
            cons[1] = fetch(pc);
            break;
    }
    
    switch ((instr >> 22) & 3) {//par3
        case 0:
            par3 = (instr2 & 65535) + 4;
            break;
        case 1:
            par3 = (instr >> 16) & 3;
            break;
        case 2:
            par3 = -3;
            cons[2] = fetch(pc);
            break;
    }
    
}

//get memory address
uint32_t* get_add(int i ){
    int32_t par = i == 1 ? par1 : ( i == 2 ? par2 : par3 );
    switch(par){
        case -1:
            return &cons[0];
            break;
        case -2:
            return &cons[1];
            break;
        case -3:
            return &cons[2];
            break;
        case 0 ... 3:
            return &regs[par];
            break;
        default:
            return &memory[par - 4];
        
    }
}

void run(uint32_t pc);

void add(){( *get_add(1) ) = ( *get_add(2) ) + ( *get_add(3) );}
void division(){( *get_add(1) ) = ( *get_add(2) ) / ( *get_add(3) );}
void mul(){( *get_add(1) ) = ( *get_add(2) ) * ( *get_add(3) );}
void ltn(){( *get_add(1) ) = ( *get_add(2) ) < ( *get_add(3) )? (uint32_t)~0:0;}
void eql(){( *get_add(1) ) = ( *get_add(2) ) == ( *get_add(3) ) ? (uint32_t)~0:0;}
void and(){( *get_add(1) ) = ( *get_add(2) ) & ( *get_add(3) );}
void or(){( *get_add(1) ) = ( *get_add(2) ) | ( *get_add(3) );}
void not(){( *get_add(1) ) = ~( *get_add(2) );}
void inc(){( *get_add(1) ) ++;}
void dec(){( *get_add(1) ) --;}
void load(){( *get_add(1) ) = ( *get_add(2) );}
void print(){printf("%d\n", ( *get_add(1) ));}
void go_to(uint32_t *pc){*pc = ( *get_add(1) );}
void IF(uint32_t *pc){if (( *get_add(1) ) == 0) *pc = ( *get_add(2) );}
void RETURN(int *running){*running = 0;}
void call(){run(*get_add(1));}



/* evaluate the last decoded instruction */
void eval(int *running, uint32_t *pc)
{
    numdisp++;
    clock_t start = clock(), diff;
    //printf("NUM: %d\n", instrNum);
    switch( instrNum )
    {
            
        case INERTIA_ADD:
            /* add */
            add();
            break;
        case INERTIA_DIV:
            /* divide */
            division();
            break;
        case INERTIA_MUL:
            /* multiply */
            mul();
            break;
            
        case INERTIA_LTN :
            //Less Than
            ltn();
            break;
        case INERTIA_EQL :
            //Equal to
            eql();
            break;
        case INERTIA_AND:
            and();
            break;
        case INERTIA_OR:
            or();
            break;
        case INERTIA_NOT:
            not();
            break;
        case INERTIA_INC:
            inc();
            break;
        case INERTIA_DEC:
            dec();
            break;
            
        case INERTIA_LOAD:
            load();
            break;
        case INERTIA_PRINT:
            print();
            break;
        case INERTIA_GOTO:
            go_to(pc);
            break;
        case INERTIA_IF:
            //printf("%d %d\n", ( *get_add(1) ), ( *get_add(2) ));
            //char w;
            //scanf("%c", &w);
            IF(pc);
            break;
        case INERTIA_RETURN:
            RETURN(running);
            break;
        case INERTIA_CALL:
            call();
            break;
    }
    tdisp += clock() - start;
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
void run(uint32_t pc)
{
    int running = 1;
    while( running )
    {
        decode( &pc );
        eval( &running , &pc );
    }
}

static inline uint32_t fgetu(){
    uint32_t a = (fgetc(f) << 24) +(fgetc(f) << 16) + (fgetc(f) << 8);
    int b = fgetc(f);
    if (b == EOF){
        printf("Failed to read file\n");
        exit(3);
    }
    return a + (uint32_t) b;
}

int main( int argc, const char * argv[] )
{
    
    if (argc == 1) printf("Please enter file name");
    
    //read in file
    
    f = fopen(argv[1], "rb");
    len_program = fgetu();
    program = (I_instr_t *)malloc(len_program * sizeof(I_instr_t));
    if(!program){
        printf("Failed to allocate instruction array\n");
        exit(1);
    }
    
    for (int i = 0; i < len_program; i ++){
        program[i].instr = fgetu();
    }
    
    fclose(f);
    
    //execute
    run(0);
    free(program);
    printf("total dispatch %d times\n", numdisp);
    printf("total dispatch time is %lu µs\n", tdisp * 1000000 / CLOCKS_PER_SEC);
    printf("total fetch time is %lu µs\n", tfetch * 1000000 / CLOCKS_PER_SEC);
    return 0;
}
