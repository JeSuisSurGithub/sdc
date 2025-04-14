#ifndef CPU_H
#define CPU_H

#include "hash.h"
#include "memory.h"
#include "parser.h"

#define STACK_SIZE 128

typedef struct CPU {
    MemoryHandler* memory_handler;
    HashMap* context;
    HashMap* constant_pool;
}CPU;

CPU* cpu_init(int memory_size);
void cpu_destroy(CPU* cpu);
void* store(MemoryHandler* handler, const char* segment_name, int pos, void* data);
void* load(MemoryHandler* handler, const char* segment_name, int pos);
void allocate_variables(CPU* cpu, Instruction** data_instructions, int data_count);
void print_data_segment(CPU* cpu);

int matches(const char* pattern, const char* string);
void* immediate_addressing(CPU* cpu, const char* operand);
void* register_addressing(CPU* cpu, const char* operand);
void* memory_direct_addressing(CPU* cpu, const char* operand);
void* register_indirect_addressing(CPU* cpu, const char* operand);
void handle_MOV(CPU* cpu, void* src, void* dest);
void* resolve_addressing(CPU* cpu, const char* operand);

void allocate_code_segment(CPU* cpu, Instruction** code_instructions, int code_count);
int handle_instruction(CPU* cpu, Instruction* instr, void* src, void* dest);
int execute_instruction(CPU* cpu, Instruction* instr);
Instruction* fetch_next_instruction(CPU* cpu);
void print_registers(CPU* cpu);
int push_value(CPU* cpu, int value);
int pop_value(CPU* cpu, int *dest);

#endif /* CPU_H */