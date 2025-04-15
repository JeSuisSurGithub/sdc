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
    if (!cpu) {
        puts("cpu_init(): malloc failed");
        return NULL;
    }

    cpu->memory_handler = memory_init(memory_size);
    if (cpu->memory_handler == NULL) {
        puts("cpu_init(): memory_init failed");
        free(cpu);
        return NULL;
    }

    cpu->context = hashmap_create();
    if (cpu->context == NULL) {
        puts("cpu_init(): hashmap_create failed");
        memory_destroy(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    cpu->constant_pool = hashmap_create();
    if (cpu->context == NULL) {
        puts("cpu_init(): hashmap_create failed ()");
        hashmap_destroy(cpu->context);
        memory_destroy(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    int* ax = calloc(1, sizeof(int));
    int* bx = calloc(1, sizeof(int));
    int* cx = calloc(1, sizeof(int));
    int* dx = calloc(1, sizeof(int));
    hashmap_insert(cpu->context, "AX", ax);
    hashmap_insert(cpu->context, "BX", bx);
    hashmap_insert(cpu->context, "CX", cx);
    hashmap_insert(cpu->context, "DX", dx);

    int* ip = calloc(1, sizeof(int));
    int* zf = calloc(1, sizeof(int));
    int* sf = calloc(1, sizeof(int));
    hashmap_insert(cpu->context, "IP", ip);
    hashmap_insert(cpu->context, "ZF", zf);
    hashmap_insert(cpu->context, "SF", sf);

    int* sp = calloc(1, sizeof(int));
    int* bp = calloc(1, sizeof(int));
    int* es = calloc(1, sizeof(int)); (*es) = -1;
    hashmap_insert(cpu->context, "SP", sp);
    hashmap_insert(cpu->context, "BP", bp);
    hashmap_insert(cpu->context, "ES", es);

    if (create_segment(cpu->memory_handler, "SS", memory_size - STACK_SIZE, STACK_SIZE) < 0) {
        puts("cpu_init(): create_segment failed");
    }
    *bp = 127;
    *sp = 127;

    return cpu;
}

void cpu_destroy(CPU* cpu)
{
    free(hashmap_get(cpu->context, "AX"));
    free(hashmap_get(cpu->context, "BX"));
    free(hashmap_get(cpu->context, "CX"));
    free(hashmap_get(cpu->context, "DX"));

    free(hashmap_get(cpu->context, "IP"));
    free(hashmap_get(cpu->context, "ZF"));
    free(hashmap_get(cpu->context, "SF"));

    int* bp = hashmap_get(cpu->context, "BP");
    int* sp = hashmap_get(cpu->context, "SP");
    while (*sp < *bp) {
        int* value = load(cpu->memory_handler, "SS", *sp++);
        free(value);
    }

    // La fonction contient déja les mesures de protections nécéssaires pour savoir il y a besoin de désallouer
    free_es_segment(cpu);
    free(hashmap_get(cpu->context, "ES"));
    free(hashmap_get(cpu->context, "BP"));
    free(hashmap_get(cpu->context, "SP"));

    Segment* DS = hashmap_get(cpu->memory_handler->allocated, "DS");
	for (int i = 0; i < DS->size; i++)
    {
        void* var = load(cpu->memory_handler, "DS", i);
        if (var != NULL) free(var);
	}
    remove_segment(cpu->memory_handler, "SS");
    remove_segment(cpu->memory_handler, "CS");
    remove_segment(cpu->memory_handler, "DS");

    for (int i = 0; i < TABLE_SIZE; i++) {
        if (cpu->constant_pool->table[i].key != NULL) {
            free(cpu->constant_pool->table[i].value);
        }
    }
    hashmap_destroy(cpu->constant_pool);
    hashmap_destroy(cpu->context);
    memory_destroy(cpu->memory_handler);
    free(cpu);
}

void* store(MemoryHandler* handler, const char* segment_name, int pos, void* data)
{
    Segment* seg = hashmap_get(handler->allocated, segment_name);

    if ((seg != NULL) && (pos <= seg->size)) {
        handler->memory[seg->start + pos] = data;
        return handler->memory[seg->start + pos];
    }

    puts("store(): unreachable address");
    return NULL;
}

void* load(MemoryHandler* handler, const char* segment_name, int pos)
{
    Segment* seg = hashmap_get(handler->allocated, segment_name);

    if ((seg != NULL) && (pos <= seg->size)) {
        return handler->memory[seg->start + pos];
    }

    puts("load(): unreachable address");
    return NULL;
}

void allocate_variables(CPU* cpu, Instruction** data_instructions, int data_count)
{
    // Pré-calcul
    unsigned int ds_size = 0;
    for (int i = 0; i < data_count; i++) {
        char* it = data_instructions[i]->operand2;
        ds_size++;
        while (*it != '\0') {
            ds_size += (*it++ == ',');
        }
    }

    Segment* SS = hashmap_get(cpu->memory_handler->allocated, "SS");
    if (create_segment(cpu->memory_handler, "DS", SS->start - ds_size, ds_size) < 0) {
        puts("allocate_variables(): create_segment failed");
    }
    unsigned int ds_idx = 0;
    for (int i = 0; i < data_count; i++) {
        char* cur = strtok(data_instructions[i]->operand2, ",");

        while (cur != NULL)
        {
            int* var = malloc(sizeof(int));
            (*var) = atoi(cur);
            store(cpu->memory_handler, "DS", ds_idx, var);
            cur = strtok(NULL, ",");
            ds_idx++;
        }
    }
}

void print_data_segment(CPU* cpu)
{
    if (cpu != NULL) {
		Segment* DS = hashmap_get(cpu->memory_handler->allocated, "DS");
   		for (int i = 0; i < DS->size; i++) {
            int* var = load(cpu->memory_handler, "DS", i);
    		if (var != NULL) {
    			printf("DS[%i] = %i | ", i, *var);
    		}
    	}
        putchar('\n');
	}
}

int matches(const char* pattern, const char* string)
{
    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED) ;
    if (result) {
        printf("Regex compilation failed for pattern: %s\n", pattern);
        return 0;
    }
    result = regexec(&regex, string, 0, NULL, 0) ;
    regfree(&regex);
    return result == 0;
}

void* immediate_addressing(CPU* cpu, const char* operand)
{
    if (matches("^[0-9]+$", operand))
    {
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

void* segment_override_addressing(CPU* cpu, const char* operand)
{
    if (!matches("^\\[(D|C|S|E)S:(A|B|C|D)X\\]$", operand)) {
        return NULL;
    }

    char segment[3] = {0};
    char reg[3] = {0};
    sscanf(operand, "[%2[^:]:%2[^]]]", segment, reg);

    Segment* seg = (Segment*)hashmap_get(cpu->memory_handler->allocated, segment);
    int* reg_val = (int*)hashmap_get(cpu->context, reg);
    if (seg == NULL || reg_val == NULL) {
        return NULL;
    }

    return load(cpu->memory_handler, segment, (*reg_val));
}

void* register_addressing(CPU* cpu, const char* operand)
{
    if (matches("^(A|B|C|D)X$", operand)) {
        return hashmap_get(cpu->context, operand);
    }
    return NULL;
}

void* memory_direct_addressing(CPU* cpu, const char* operand)
{
    if (matches("^\\[[0-9]+\\]$", operand)) {
        int addr = 0;
        sscanf(operand, "[%i]", &addr);
        return load(cpu->memory_handler, "DS", addr);
    }
    return NULL;
}

void* register_indirect_addressing(CPU* cpu, const char* operand)
{
    if (matches("^\\[(A|B|C|D)X\\]$", operand)) {
        char addr[3] = {0};
        sscanf(operand, "[%02s]", addr);

        return load(cpu->memory_handler, "DS", *(int*)hashmap_get(cpu->context, addr));
    }
    return NULL;
}

void handle_MOV(CPU* cpu, void* src, void* dest)
{
    int* isrc = (int*)src;
    int* idest = (int*)dest;
    *(idest) = *(isrc);
}

void* resolve_addressing(CPU* cpu, const char* operand) {
    void* imm = immediate_addressing(cpu, operand);
    if (imm != NULL) return imm;

    void* reg = register_addressing(cpu, operand);
    if (reg != NULL) return reg;

    void* direct = memory_direct_addressing(cpu, operand);
    if (direct != NULL) return direct;

    void* indirect = register_indirect_addressing(cpu, operand);
    if (indirect != NULL) return indirect;

    void* segment_override = segment_override_addressing(cpu, operand);
    if (segment_override != NULL) return segment_override;

    puts("resolve_addressing(): operand doesn't match with any mode");
    return NULL;
}

void allocate_code_segment(CPU* cpu, Instruction** code_instructions, int code_count)
{
    if ((cpu == NULL) || (code_instructions == NULL) || code_count <= 0) {
        return;
    }

    if (create_segment(cpu->memory_handler, "CS", 0, code_count) < 0) {
        puts("allocate_code_segment(): create_segment failed");
    }

    for (int i = 0; i < code_count; i++) {
        store(cpu->memory_handler, "CS", i, code_instructions[i]);
    }

    int* ip = (int*)hashmap_get(cpu->context, "IP");
    *ip = 0;
}

int free_es_segment(CPU* cpu)
{
    int* es = (int*)hashmap_get(cpu->context, "ES");
    if (es == NULL || *es == -1) return -1;

    Segment* seg = (Segment*)hashmap_get(cpu->memory_handler->allocated, "ES");
    if (seg == NULL) return -2;

    for (int i = 0; i < seg->size; i++)
    {
        int index = seg->start + i;
        if (cpu->memory_handler->memory[index]) {
            free(cpu->memory_handler->memory[index]);
            cpu->memory_handler->memory[index] = NULL;
        }
    }

    if (remove_segment(cpu->memory_handler, "ES") < 0) return -3;

    *es = -1;
    return 0;
}

int alloc_es_segment(CPU* cpu)
{
    int* ax = (int*)hashmap_get(cpu->context, "AX");
    int* bx = (int*)hashmap_get(cpu->context, "BX");
    int* es = (int*)hashmap_get(cpu->context, "ES");
    int* zf = (int*)hashmap_get(cpu->context, "ZF");

    if (!ax || !bx || !es || !zf) return -1;

    int size = *ax;
    int strategy = *bx;

    int start = find_free_address_strategy(cpu->memory_handler, size, strategy);
    if (start < 0) {
        *zf = 1;
        return -1;
    }

    if (create_segment(cpu->memory_handler, "ES", start, size) < 0) {
        *zf = 1;
        return -1;
    }

    for (int i = 0; i < size; ++i) {
        int* zero = malloc(sizeof(int));
        *zero = 0;
        store(cpu->memory_handler, "ES", i, zero);
    }

    *es = start;
    *zf = 0;
    return 0;
}

int handle_instruction(CPU* cpu, Instruction* instr, void* src, void* dest)
{
    if ((cpu == NULL) || (instr == NULL)) {
        return -1;
    }

    int* ip = (int*)hashmap_get(cpu->context, "IP");
    int* zf = (int*)hashmap_get(cpu->context, "ZF");
    int* sf = (int*)hashmap_get(cpu->context, "SF");

    if (strcmp(instr->mnemonic, "MOV") == 0) {
        if (!src || !dest) {
            puts("handle_instruction(): MOV Invalid operand");
            return -2;
        }
        handle_MOV(cpu, src, dest);
    }
    else if (strcmp(instr->mnemonic, "ADD") == 0) {
        if (!src || !dest) {
            puts("handle_instruction(): ADD Invalid operand");
            return -3;
        }
        *(int*)dest += *(int*)src;
    }
    else if (strcmp(instr->mnemonic, "CMP") == 0) {
        if (!src || !dest) {
            puts("handle_instruction(): CMP Invalid operand");
            return -4;
        }
        int result = *(int*)dest - *(int*)src;
        *zf = (result == 0) ? 1 : 0;
        *sf = (result < 0) ? 1 : 0;
    }
    else if (strcmp(instr->mnemonic, "JMP") == 0) {
        if (!dest) {
            puts("handle_instruction(): JMP Invalid operand");
            return -5;
        }
        *ip = *(int*)dest;
    }
    else if (strcmp(instr->mnemonic, "JZ") == 0) {
        if (!dest || !zf) {
            puts("handle_instruction(): JZ Invalid operand");
            return -6;
        }
        if (*zf == 1) *ip = *(int*)dest;
    }
    else if (strcmp(instr->mnemonic, "JNZ") == 0) {
        if (!dest || !zf) {
            puts("handle_instruction(): JNZ Invalid operand");
            return -7;
        }
        if (*zf == 0) *ip = *(int*)dest;
    }
    else if (strcmp(instr->mnemonic, "HALT") == 0) {
        Segment* cs_seg = (Segment *)hashmap_get(cpu->memory_handler->allocated, "CS");
        if (cs_seg) *ip = cs_seg->size;
    }
    else if (strcmp(instr->mnemonic, "PUSH") == 0) {
        if (!src) {
            int* ax = (int*)hashmap_get(cpu->context, "AX");
            if (!ax) {
                puts("handle_instruction(): PUSH Invalid operand");
                return -8;
            }
            return push_value(cpu, *ax);
        }
        return push_value(cpu, *(int *)src);
    }
    else if (strcmp(instr->mnemonic, "POP") == 0) {
        int value;
        if (pop_value(cpu, &value)) return -1;

        if (!dest) {
            int* ax = (int*)hashmap_get(cpu->context, "AX");
            if (!ax) {
                puts("handle_instruction(): POP Invalid operand");
                return -9;
            }
            *ax = value;
        } else {
            *(int*)dest = value;
        }
    }
    else if (strcmp(instr->mnemonic, "ALLOC") == 0) {
   		return alloc_es_segment(cpu);
	}
    else if (strcmp(instr->mnemonic, "FREE") == 0) {
    	return free_es_segment(cpu);
	}
    else {
        puts("handle_instruction(): unrecognized instruction");
        return -10;
    }

    return 0;
}

int execute_instruction(CPU* cpu, Instruction* instr)
{
    if ((cpu == NULL) || (instr == NULL)){
        puts("execute_instruction(): invalid arguments");
        return -1;
    }

    void* src = NULL;
    void* dest = NULL;

    if (instr->operand1 != NULL && strcmp(instr->operand1, "") != 0) {
        dest = resolve_addressing(cpu, instr->operand1);
        if (!dest &&
            strcmp(instr->mnemonic, "JMP") != 0 &&
            strcmp(instr->mnemonic, "JZ") != 0 &&
            strcmp(instr->mnemonic, "JNZ") != 0)
        {
            puts("execute_instruction(): Invalid operand 1");
            return -2;
        }
    }

    if (instr->operand2 != NULL && strcmp(instr->operand2, "") != 0) {
        src = resolve_addressing(cpu, instr->operand2);
        if (!src && strcmp(instr->mnemonic, "CMP") != 0)
        {
            puts("execute_instruction(): Invalid operand 2");
            return -3;
        }
    }

    return handle_instruction(cpu, instr, src, dest);
}

Instruction* fetch_next_instruction(CPU* cpu)
{
    if (cpu == NULL) return NULL;

    int* ip = (int*)hashmap_get(cpu->context, "IP");
    if (ip == NULL) return NULL;

    Segment* cs_seg = (Segment*)hashmap_get(cpu->memory_handler->allocated, "CS");
    if (cs_seg == NULL) return NULL;

    if (*ip < 0 || *ip >= cs_seg->size) return NULL;

    Instruction* instr = (Instruction*)load(cpu->memory_handler, "CS", *ip);
    if (instr == NULL) return NULL;

    (*ip)++;

    return instr;
}

void print_registers(CPU* cpu)
{
    if (cpu == NULL) return;

    const char* reg_names[] = {
        "AX",
        "BX",
        "CX",
        "DX",
        "IP",
        "ZF",
        "SF",
        "SP",
        "BP",
        "ES"};
    for (int i = 0; i < (sizeof(reg_names)/sizeof(reg_names[0])); i++) {
        int* val = (int*)hashmap_get(cpu->context, reg_names[i]);
        if (val) {
            printf("%s: %d | ", reg_names[i], *val);
        }
    }
    putchar('\n');
}

int push_value(CPU* cpu, int value)
{
    if (cpu == NULL) return -1;

    int* sp = hashmap_get(cpu->context, "SP");
    if (sp == NULL) return -2;

    if (*sp < 0) {
        printf("push_value(): stack overflow!\n");
        return -3;
    }

    int* val_mem = (int*)malloc(sizeof(int));
    if (val_mem == NULL) return -4;
    (*val_mem) = value;
    store(cpu->memory_handler, "SS", *sp, val_mem);

    (*sp)--;

    return 0;
}

int pop_value(CPU* cpu, int* dest)
{
    if (cpu == NULL || dest == NULL) return -1;

    int* sp = (int*)hashmap_get(cpu->context, "SP");
    int* bp = (int*)hashmap_get(cpu->context, "BP");
    if (sp == NULL || bp == NULL) return -2;

    if (*sp >= *bp) {
        printf("pop_value(): stack underflow!\n");
        return -3;
    }

    (*sp)++;

    int* val_mem = load(cpu->memory_handler, "SS", *sp);
    if (val_mem == NULL) return -4;

    *dest = *val_mem;
    free(val_mem);

    return 0;
}