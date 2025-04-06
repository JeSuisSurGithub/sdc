#ifndef CPU_H
#define CPU_H

#include "hash.h"
#include "memory.h"
#include "parser.h"

typedef struct CPU{
    MemoryHandler* memory_handler;
    HashMap* context;
    HashMap* constant_pool;
}CPU;

CPU* cpu_init(int memory_size);
void cpu_destroy(CPU* cpu);
void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data);
void* load(MemoryHandler *handler, const char *segment_name, int pos);
void allocate_variables(CPU *cpu, Instruction** data_instructions, int data_count);
void print_data_segment(CPU *cpu);
int matches(const char* pattern, const char* string);
void* immediate_addressing(CPU *cpu, const char *operand);
void* register_addressing(CPU *cpu, const char *operand);

#endif /* CPU_H */