#include "lc3_vm.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// uncomment if compiling for a big-endian architecture
//#ifndef TARGET_BIG_ENDIAN

uint16_t sign_extend(uint16_t x, int bit_count)
{
    // if negative with bit_count...
    if ((x >> (bit_count - 1)) & 1)
    {
        // ... replace zeros left to bit_count with ones,
        // so that it represents the same negative value
        x |= (0xFFFF << bit_count);
    }

    return x;
}

void mem_write(t_lc3_vm *vm, uint16_t address, uint16_t value)
{
    vm->memory[address] = value;
}

uint16_t mem_read(t_lc3_vm *vm, uint16_t address)
{
    uint16_t *memory = vm->memory;

    if (address == MR_KBSR)
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBSR] = 0;
        }
    }

    return memory[address];
}

void update_cond_flags(t_lc3_vm *vm, uint16_t r)
{
    uint16_t *reg = vm->reg;

    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) // if negative, i.e. leftmost bit == 1
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

void add(t_lc3_vm *vm, uint16_t instr)
{
    uint16_t *reg = vm->reg;

    // destination register (DR)
    uint16_t r0 = THREE_BITS(instr >> 9);

    // first operand (SR1)
    uint16_t r1 = THREE_BITS(instr >> 6);

    // immediate mode flag
    uint16_t imm_flag = ONE_BIT(instr >> 5);

    if (imm_flag)
    {
        // sign-extended imm5 to 16 bits
        uint16_t imm5 = sign_extend(FIVE_BITS(instr), 5);
        reg[r0] = reg[r1] + imm5;
    }
    else
    {
        // second operand (SR2)
        uint16_t r2 = THREE_BITS(instr);
        reg[r0] = reg[r1] + reg[r2];
    }

    update_cond_flags(vm, r0);
}

void and(t_lc3_vm *vm, uint16_t instr)
{
    uint16_t *reg = vm->reg;

    // destination register (DR)
    uint16_t r0 = THREE_BITS(instr >> 9);

    // first operand (SR1)
    uint16_t r1 = THREE_BITS(instr >> 6);

    // immediate mode flag
    uint16_t imm_flag = ONE_BIT(instr >> 5);

    if (imm_flag)
    {
        // sign-extended imm5 to 16 bits
        uint16_t imm5 = sign_extend(FIVE_BITS(instr), 5);
        reg[r0] = reg[r1] & imm5;
    }
    else
    {
        // second operand (SR2)
        uint16_t r2 = THREE_BITS(instr);
        reg[r0] = reg[r1] & reg[r2];
    }

    update_cond_flags(vm, r0);
}

void br(t_lc3_vm *vm, uint16_t instr)
{
    uint16_t *reg = vm->reg;

    uint16_t pc_offset = sign_extend(NINE_BITS(instr), 9);
    uint16_t cond_flag = THREE_BITS(instr >> 9); // contains N, Z, P bit by bit

    if (cond_flag & reg[R_COND]) // if any of them matches
    {
        reg[R_PC] += pc_offset;
    }
}

void jmp(t_lc3_vm *vm, uint16_t instr)
{
    uint16_t *reg = vm->reg;

    uint16_t baseR = THREE_BITS(instr >> 6);
    reg[R_PC] = reg[baseR];
}

void jsr(t_lc3_vm *vm, uint16_t instr)
{
    uint16_t *reg = vm->reg;

    reg[R_R7] = reg[R_PC];
    uint16_t flag = ONE_BIT(instr >> 11);

    if (flag)
    {
        uint16_t pc_offset = sign_extend(ELEVEN_BITS(instr), 11);
        reg[R_PC] += pc_offset;
    }
    else
    {
        uint16_t baseR = THREE_BITS(instr >> 6);
        reg[R_PC] = reg[baseR];
    }
}

void ld(t_lc3_vm *vm, uint16_t instr)
{
    uint16_t *reg = vm->reg;

    uint16_t pc_offset = sign_extend(NINE_BITS(instr), 9);
    uint16_t dr = THREE_BITS(instr >> 9);

    reg[dr] = mem_read(vm, reg[R_PC] + pc_offset);

    update_cond_flags(vm, dr);
}

void ldi(t_lc3_vm *vm, uint16_t instr)
{
    uint16_t *reg = vm->reg;

    uint16_t pc_offset = sign_extend(NINE_BITS(instr), 9);
    uint16_t dr = THREE_BITS(instr >> 9);

    reg[dr] = mem_read(vm, mem_read(vm, reg[R_PC] + pc_offset));

    update_cond_flags(vm, dr);
}

void ldr(t_lc3_vm *vm, uint16_t instr)
{
    uint16_t *reg = vm->reg;

    uint16_t pc_offset = sign_extend(SIX_BITS(instr), 6);
    uint16_t baseR = THREE_BITS(instr >> 6);
    uint16_t dr = THREE_BITS(instr >> 9);

    reg[dr] = mem_read(vm, reg[baseR] + pc_offset);

    update_cond_flags(vm, dr);
}

void lea(t_lc3_vm *vm, uint16_t instr)
{
    uint16_t *reg = vm->reg;

    uint16_t pc_offset = sign_extend(NINE_BITS(instr), 9);
    uint16_t dr = THREE_BITS(instr >> 9);

    reg[dr] = reg[R_PC] + pc_offset;

    update_cond_flags(vm, dr);
}

void not(t_lc3_vm *vm, uint16_t instr)
{
    uint16_t *reg = vm->reg;

    uint16_t dr = THREE_BITS(instr >> 9);
    uint16_t sr = THREE_BITS(instr >> 6);

    reg[dr] = ~reg[sr];

    update_cond_flags(vm, dr);
}

void st(t_lc3_vm *vm, uint16_t instr)
{
    uint16_t *reg = vm->reg;

    uint16_t pc_offset = sign_extend(NINE_BITS(instr), 9);
    uint16_t sr = THREE_BITS(instr >> 9);

    mem_write(vm, reg[R_PC] + pc_offset, reg[sr]);
}

void sti(t_lc3_vm *vm, uint16_t instr)
{
    uint16_t *reg = vm->reg;

    uint16_t pc_offset = sign_extend(NINE_BITS(instr), 9);
    uint16_t sr = THREE_BITS(instr >> 9);

    mem_write(vm, mem_read(vm, reg[R_PC] + pc_offset), reg[sr]);
}

void str(t_lc3_vm *vm, uint16_t instr)
{
    uint16_t *reg = vm->reg;

    uint16_t offset = sign_extend(SIX_BITS(instr), 6);
    uint16_t baseR = THREE_BITS(instr >> 6);
    uint16_t sr = THREE_BITS(instr >> 9);

    mem_write(vm, reg[baseR] + offset, reg[sr]);
}

void trap_getc(t_lc3_vm *vm)
{
    vm->reg[R_R0] = (uint16_t) getchar();
}

void trap_out(t_lc3_vm *vm)
{
    putc((char) vm->reg[R_R0], stdout);
    fflush(stdout);
}

void trap_puts(t_lc3_vm *vm)
{
    uint16_t *c = &(vm->memory[vm->reg[R_R0]]);

    while (*c) // while not \0
    {
        putc((char) *c, stdout);
        c++;
    }

    fflush(stdout);
}

void trap_in(t_lc3_vm *vm)
{
    printf("Enter one character: ");
    fflush(stdout);
    char c = getchar();
    putc(c, stdout);
    fflush(stdout);
}

void trap_putsp(t_lc3_vm *vm)
{
    uint16_t *c = &(vm->memory[vm->reg[R_R0]]);

    while (*c) // while not \0
    {
        char c1 = EIGHT_BITS(*c);
        putc(c1, stdout);
        char c2 = (*c) >> 8;
        putc(c2, stdout);
        putc(c2, stdout);
        c++;
    }

    fflush(stdout);
}

int trap(t_lc3_vm *vm, uint16_t instr)
{
    int keep_running = 1;

    switch (EIGHT_BITS(instr))
    {
        case TRAP_GETC:
            trap_getc(vm);
            break;
        case TRAP_OUT:
            trap_out(vm);
            break;
        case TRAP_PUTS:
            trap_puts(vm);
            break;
        case TRAP_IN:
            trap_in(vm);
            break;
        case TRAP_PUTSP:
            trap_putsp(vm);
            break;
        case TRAP_HALT:
            puts("Halting...");
            fflush(stdout);
            keep_running = 0;
            break;   
    }

    return keep_running;
}

#ifndef TARGET_BIG_ENDIAN
uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}
#endif

void read_image_file(t_lc3_vm *vm, FILE *file)
{
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);

#ifndef TARGET_BIG_ENDIAN
    origin = swap16(origin); // big endian -> little endian
#endif

    uint16_t max_read = UINT16_MAX - origin;
    uint16_t *p = vm->memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

#ifndef TARGET_BIG_ENDIAN
    // big endian -> little endian
    while(read-- > 0)
    {
        *p = swap16(*p);
        p++;
    }
#endif
}

int read_image(t_lc3_vm *vm, const char *image_path)
{
    FILE *file = fopen(image_path, "rb");
    if (!file)
    {
        return 0;
    }

    read_image_file(vm, file);

    fclose(file);
    return 1;
}

void free_vm(t_lc3_vm *vm)
{
    if (vm)
    {
        free(vm);
    }
}

t_lc3_vm *load_vm(const char *path)
{
    t_lc3_vm *vm = (t_lc3_vm*) malloc(sizeof(t_lc3_vm));

    if (!read_image(vm, path))
    {
        free_vm(vm);
        return NULL;
    }

    vm->running = 0;

    return vm;
}

void run_vm(t_lc3_vm *vm)
{
    // set the PC at the starting position
    enum { PC_START = 0x3000 };
    vm->reg[R_PC] = PC_START;

    vm->running = 1;
    while (vm->running)
    {
        // fetch instruction
        uint16_t instr = mem_read(vm, vm->reg[R_PC]++);
        //printf("%#010x\n", instr);

        // get 4 leftmost bits, that's the opcode
        // the rest of the instruction are the arguments
        uint16_t op = instr >> 12; 

        switch (op)
        {
            case OP_ADD:
                add(vm, instr);
                break;
            case OP_AND:
                and(vm, instr);
                break;
            case OP_NOT:
                not(vm, instr);
                break;
            case OP_BR:
                br(vm, instr);
                break;
            case OP_JMP:
                jmp(vm, instr);
                break;
            case OP_JSR:
                jsr(vm, instr);
                break;
            case OP_LD:
                ld(vm, instr);
                break;
            case OP_LDI:
                ldi(vm, instr);
                break;
            case OP_LDR:
                ldr(vm, instr);
                break;
            case OP_LEA:
                lea(vm, instr);
                break;
            case OP_ST:
                st(vm, instr);
                break;
            case OP_STI:
                sti(vm, instr);
                break;
            case OP_STR:
                str(vm, instr);
                break;
            case OP_TRAP:
                vm->running = trap(vm, instr);
                break;
            case OP_RES:
            case OP_RTI:
            default:
                // bad opcode
                break;
        }
    }
}

void shutdown_vm(t_lc3_vm *vm)
{
    free_vm(vm);
}