#ifndef HEADER_LC3_VM
#define HEADER_LC3_VM

#include "win_tools.h"

#include <inttypes.h>

// registers
enum
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,
    R_COND,
    R_COUNT  // total amount of registers in the enum, nice trick
};

// opcodes
enum
{
    OP_BR = 0,  // branch
    OP_ADD,     // add
    OP_LD,      // load
    OP_ST,      // store
    OP_JSR,     // jump register
    OP_AND,     // bitwise and
    OP_LDR,     // load register
    OP_STR,     // store register
    OP_RTI,     // unused
    OP_NOT,     // bitwise not
    OP_LDI,     // load indirect
    OP_STI,     // store indirect
    OP_JMP,     // jump
    OP_RES,     // reserved (unused)
    OP_LEA,     // load effective address
    OP_TRAP     // execute trap
};

// condition flags
enum
{
    FL_POS = 1 << 0,    // positive (P)
    FL_ZRO = 1 << 1,    // zero (Z)
    FL_NEG = 1 << 2     // negative (N)
};

// trap routines
enum
{
    TRAP_GETC   = 0x20, // get char from keyboard, not echoed in terminal
    TRAP_OUT    = 0x21, // output char
    TRAP_PUTS   = 0x22, // output word string
    TRAP_IN     = 0x23, // get char from keyboard, echoed in terminal
    TRAP_PUTSP  = 0x24, // output byte string
    TRAP_HALT   = 0x25, // stop program
};

// memory mapped registers
enum
{
    MR_KBSR = 0xFE00,   // keyboard status
    MR_KBDR = 0xFE02,   // keyboard data
};

#define ONE_BIT(a)      (a) & 0x1
#define THREE_BITS(a)   (a) & 0x7
#define FIVE_BITS(a)    (a) & 0x1F
#define SIX_BITS(a)     (a) & 0x3F
#define EIGHT_BITS(a)   (a) & 0xFF
#define NINE_BITS(a)    (a) & 0x1FF
#define ELEVEN_BITS(a)  (a) & 0x7FF

typedef struct 
{
    uint16_t memory[UINT16_MAX];
    uint16_t reg[R_COUNT];

    int running;
} t_lc3_vm;

t_lc3_vm *load_vm(const char *path);
void run_vm(t_lc3_vm *vm);
void shutdown_vm(t_lc3_vm *vm);

#endif