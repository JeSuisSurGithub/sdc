#include "cpu.h"
#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CPU* cpu_init(int memory_size)
{
    CPU* cpu = (CPU*)malloc(sizeof(CPU));
    cpu->memory_handler = memory_init(memory_size);
    cpu->context = hashmap_create();

    int* ax = calloc(1, sizeof(int));
    int* bx = calloc(1, sizeof(int));
    int* cx = calloc(1, sizeof(int));
    int* dx = calloc(1, sizeof(int));
    hashmap_insert(cpu->context, "AX", ax);
    hashmap_insert(cpu->context, "BX", bx);
    hashmap_insert(cpu->context, "CX", cx);
    hashmap_insert(cpu->context, "DX", dx);
    return cpu;
}

void cpu_destroy(CPU* cpu)
{
    free(hashmap_get(cpu->context, "AX"));
    free(hashmap_get(cpu->context, "BX"));
    free(hashmap_get(cpu->context, "CX"));
    free(hashmap_get(cpu->context, "DX"));

    hashmap_destroy(cpu->context);
    memory_destroy(cpu->memory_handler);
    free(cpu);
}

void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data)
{
    Segment* seg = hashmap_get(handler->allocated, segment_name);

    if ((seg != NULL) && (pos <= seg->size)) {
        handler->memory[seg->start + pos] = data;
        return handler->memory[seg->start + pos];
    }

    return NULL;
}

void* load(MemoryHandler *handler, const char *segment_name, int pos)
{
    Segment* seg = hashmap_get(handler->allocated, segment_name);

    if ((seg != NULL) && (pos <= seg->size)) {
        return handler->memory[seg->start + pos];
    }

    return NULL;
}

void allocate_variables(CPU *cpu, Instruction** data_instructions, int data_count)
{
    unsigned ds_size = 0;
    for (int i = 0; i < data_count; i++) {
        char* arr = data_instructions[i]->operand2;

        unsigned ins_size = 0;
        char* cur = strtok(arr, ",");

        while (cur != NULL)
        {
            ins_size++;
            cpu->memory_handler->memory[ds_size + ins_size] = malloc(sizeof(int));
            *(int*)(cpu->memory_handler->memory[ds_size + ins_size]) = atoi(cur);
            char* cur = strtok(NULL, ",");
        }

        ds_size += ins_size;
    }
    create_segment(cpu->memory_handler, "DS", 0, ds_size * sizeof(int));
}

void print_data_segment(CPU *cpu)
{
    Segment* DS = hashmap_get(cpu->memory_handler->allocated, "DS");
    for (int i = DS->start; i < (DS->start + DS->size); i++) {
        printf("%i\n", *(int*)(cpu->memory_handler->memory[i]));
    }
}