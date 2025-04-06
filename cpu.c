 #include "cpu.h"
#include "hash.h"
#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

CPU* cpu_init(int memory_size)
{
    CPU* cpu = (CPU*)malloc(sizeof(CPU));
    cpu->memory_handler = memory_init(memory_size);
    cpu->context = hashmap_create();
    cpu->constant_pool = hashmap_create();

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

    Segment* DS = hashmap_get(cpu->memory_handler->allocated, "DS");
	for (int i = 0; i < (DS->size / sizeof(int)); i++)
    {
    	if (cpu->memory_handler->memory[i] != NULL) {
    		free(cpu->memory_handler->memory[i]);
    	}
	}
    remove_segment(cpu->memory_handler, "DS");

    hashmap_destroy(cpu->constant_pool);
    hashmap_destroy(cpu->context);
    memory_destroy(cpu->memory_handler);
    free(cpu);
}

void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data)
{
    Segment* seg = hashmap_get(handler->allocated, segment_name);

    if ((seg != NULL) && (pos <= seg->size)) {
        if (handler->memory[seg->start + pos] != NULL) {
            free(handler->memory[seg->start + pos]);
            handler->memory[seg->start + pos] = NULL;
        }
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
            cpu->memory_handler->memory[ds_size + ins_size] = malloc(sizeof(int));
            *(int*)(cpu->memory_handler->memory[ds_size + ins_size]) = atoi(cur);
            cur = strtok(NULL, ",");
            ins_size++;
        }

        ds_size += ins_size;
    }
    create_segment(cpu->memory_handler, "DS", 0, ds_size * sizeof(int));
}

void print_data_segment(CPU *cpu)
{
    if (cpu != NULL) {
		Segment* DS = hashmap_get(cpu->memory_handler->allocated, "DS");
   		for (int i = DS->start; i < (DS->start + DS->size); i++) {
    		if (cpu->memory_handler->memory[i] != NULL) {
    			printf("%i\n", *(int*)(cpu->memory_handler->memory[i]));
    		}
    	}
	}
}

int matches(const char* pattern, const char* string)
{
    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED) ;
    if (result) {
        fprintf(stderr, "Regex compilation failed for pattern: %s\n", pattern);
        return 0;
    }
    result = regexec(&regex, string, 0, NULL, 0) ;
    regfree (&regex) ;
    return result == 0;
}

void* immediate_addressing(CPU *cpu, const char *operand)
{
    if (matches("^[0-9]+$", operand)) {
        int n = atoi(operand);
        int* val = hashmap_get(cpu->constant_pool, operand);
        if (val == NULL) {
            int* pn = malloc(sizeof(int)); (*pn) = n;
            hashmap_insert(cpu->constant_pool, operand, pn);
            return pn;
        } else {
            return val;
        }
    }
    return NULL;
}

void* register_addressing(CPU *cpu, const char *operand)
{
    if (matches("^(A|B|C|D)X$", operand)) {
        return hashmap_get(cpu->context, operand);
    }
    return NULL;
}
