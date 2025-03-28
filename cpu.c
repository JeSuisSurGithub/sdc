#include "cpu.h"
#include "memory.h"

#include <stdlib.h>

CPU* cpu_init(int memory_size)
{
    CPU* cpu = (CPU*)malloc(sizeof(CPU));
    cpu->memory_handler = memory_init(memory_size);
    cpu->context = hashmap_create();

    int* ax = malloc(sizeof(int)); (*ax) = 0;
    int* bx = malloc(sizeof(int)); (*bx) = 0;
    int* cx = malloc(sizeof(int)); (*cx) = 0;
    int* dx = malloc(sizeof(int)); (*dx) = 0;
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

void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data){
    Segment* seg = hashmap_get(handler->allocated, segment_name);
    if (seg != NULL && pos <=seg->size){
        handler->memory[seg->start + pos] = data;
    } else {
        // ????
        find_free_segment(MemoryHandler* handler, int start, int
size, Segment** prev)
    }
    handler->total_size = handler->total_size + 1;

    return handler->memory[seg->start + pos];
}

void* load(MemoryHandler *handler, const char *segment_name, int pos) {
    Segment* seg = hashmap_get(handler->allocated, segment_name);
    if (seg != NULL && pos <=seg->size) {
        return handler->memory[seg->start + pos];
    }
}

void allocate_variables(CPU *cpu, Instruction** data_instructions, int data_count)
{
    int total_size = 0;
    for (int i = 0; i < data_count; i++) {
        int arr_size = 1;
        char* var = data_instructions[i]->operand2;
        while (*var++ != '\0') {
            arr_size += (*var == ',');
        }
        total_size += arr_size;
    }
    total_size *= sizeof(int);

    int accu = 0;
    if (create_segment(cpu->handler, "DS", 0, total_size) != -1)
    {
        for (int i = 0; i < data_count; i++) {
            int arr_size = 1;
            char* var = data_instructions[i]->operand2;
            while (*var++ != '\0') {
                arr_size += (*var == ',');
            }
            for (int j = 0; j < arr_size; j++) {
                cpu->memory_handler->memory[total_size + j] = malloc(sizeof(int))
            }
            accu += arr_size;
        }
    } else {
        //
    }
}

void print_data_segment(CPU *cpu)
{
    Segment* DS = hashmap_get(cpu->memory_handler, "DS");
    for (int i = DS->start; i < (DS->start + DS->size); i++) {
        printf("%i\n", (*cpu->memory[i]))
    }
}