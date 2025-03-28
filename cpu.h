#ifndef CPU_H
#define CPU_H

#include "hash.h"
#include "memory.h"
#include "parser.h"

typedef struct CPU{
    MemoryHandler* memory_handler;
    HashMap* context;
}CPU;

CPU* cpu_init(int memory_size);
void cpu_destroy(CPU* cpu);

#endif /* CPU_H */