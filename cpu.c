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
        return NULL;
    }

    cpu->memory_handler = memory_init(memory_size);
    if (!cpu->memory_handler) {
        free(cpu);
        return NULL;
    }

    cpu->context = hashmap_create();
    if (!cpu->context) {
        memory_destroy(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    cpu->constant_pool = hashmap_create();
    if (!cpu->context) {
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
    hashmap_insert(cpu->context, "SP", sp);
    hashmap_insert(cpu->context, "BP", bp);

    create_segment(cpu->memory_handler, "SS", 0, 128);
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

    Segment* DS = hashmap_get(cpu->memory_handler->allocated, "DS");
	for (int i = 0; i < (DS->size / sizeof(int)); i++)
    {
    	if (cpu->memory_handler->memory[i] != NULL) {
    		free(cpu->memory_handler->memory[i]);
    	}
	}
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
    			printf("DS[%i] = %i\n", i, *(int*)(cpu->memory_handler->memory[i]));
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

void* memory_direct_addressing(CPU *cpu, const char *operand)
{
    if (matches("^\\[[0-9]+\\]$", operand)) {
        int addr = 0;
        sscanf(operand, "[%i]", &addr);
        return cpu->memory_handler->memory[addr];
    }
    return NULL;
}

void* register_indirect_addressing(CPU *cpu, const char* operand)
{
    if (matches("^\\[(A|B|C|D)X\\]$", operand)) {
        char addr[3] = {0};
        sscanf(operand, "[%02s]", addr);
        return cpu->memory_handler->memory[*(int*)hashmap_get(cpu->context, addr)];
    }
    return NULL;
}

void handle_MOV(CPU* cpu, void* src, void* dest)
{
    int* isrc = (int*)src;
    int* idest = (int*)dest;
    *(idest) = *(isrc);
}

CPU* setup_test_environment()
{
    CPU* cpu = cpu_init(1024);
    if (!cpu) {
        printf("Error: CPU initialition failed\n");
        return NULL;
    }

    int* ax = (int*)hashmap_get(cpu->context, "AX");
    int* bx = (int*)hashmap_get(cpu->context, "BX");
    int* cx = (int*)hashmap_get(cpu->context, "CX");
    int* dx = (int*)hashmap_get(cpu->context, "DX");

    *ax = 3;
    *bx = 6;
    *cx = 100;
    *dx = 5;

    if (!hashmap_get(cpu->memory_handler->allocated, "DS")) {
        create_segment(cpu->memory_handler, "DS", 0, 10 * sizeof(int));

        for (int i = 0; i < 10; i++) {
            int* value = (int*)malloc(sizeof(int));
            *value = i * 10 + 5;
            store(cpu->memory_handler, "DS", i, value);
        }
    }

    printf("Test environnment initialized\n");
    return cpu;
}

void* resolve_addressing(CPU *cpu, const char *operand)
{
    void* imm = immediate_addressing(cpu, operand);
    if (imm != NULL) return imm;

    void* reg = register_addressing(cpu, operand);
    if (reg != NULL) return reg;

    void* direct = memory_direct_addressing(cpu, operand);
    if (direct != NULL) return direct;

    void* indirect = register_indirect_addressing(cpu, operand);
    if (indirect != NULL) return indirect;

    return NULL;
}

void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count)
{
    if (!cpu || !code_instructions || code_count <= 0) return;

    create_segment(cpu->memory_handler, "CS", 0, code_count);

    for (int i = 0; i < code_count; i++) {
        store(cpu->memory_handler, "CS", i, code_instructions[i]);
    }

    int* ip = (int*)hashmap_get(cpu->context, "IP");
    *ip = 0;
}

int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest)
{
    if (!cpu || !instr) {
        return 0;
    }

    int *ip = (int *)hashmap_get(cpu->context, "IP");
    int *zf = (int *)hashmap_get(cpu->context, "ZF");
    int *sf = (int *)hashmap_get(cpu->context, "SF");

    if (strcmp(instr->mnemonic, "MOV") == 0) {
        if (!src || !dest) return 0;
        *(int *)dest = *(int *)src;
    }
    else if (strcmp(instr->mnemonic, "ADD") == 0) {
        if (!src || !dest) return 0;
        *(int *)dest += *(int *)src;
    }
    else if (strcmp(instr->mnemonic, "CMP") == 0) {
        if (!src || !dest) return 0;
        int result = *(int*)dest - *(int*)src;
        *zf = (result == 0) ? 1 : 0;
        *sf = (result < 0) ? 1 : 0;
    }
    else if (strcmp(instr->mnemonic, "JMP") == 0) {
        if (!src) return 0;
        *ip = *(int *)src;
    }
    else if (strcmp(instr->mnemonic, "JZ") == 0) {
        if (!src || !zf) return 0;
        if (*zf == 1) *ip = *(int *)src;
    }
    else if (strcmp(instr->mnemonic, "JNZ") == 0) {
        if (!src || !zf) return 0;
        if (*zf == 0) *ip = *(int *)src;
    }
    else if (strcmp(instr->mnemonic, "HALT") == 0) {
        Segment *cs_seg = (Segment *)hashmap_get(cpu->memory_handler->allocated, "CS");
        if (cs_seg) *ip = cs_seg->size;
    }
    else if (strcmp(instr->mnemonic, "PUSH") == 0) {
        if (!src) {
            int *ax = (int *)hashmap_get(cpu->context, "AX");
            if (!ax) return 0;
            return push_value(cpu, *ax);
        }
        return push_value(cpu, *(int *)src);
    }
    else if (strcmp(instr->mnemonic, "POP") == 0) {
        int value;
        if (pop_value(cpu, &value)) return 0;

        if (!dest) {
            int *ax = (int *)hashmap_get(cpu->context, "AX");
            if (!ax) return 0;
            *ax = value;
        } else {
            *(int *)dest = value;
        }
        return 1;
    }
    else {
        return 0; // Instruction non reconnue
    }

    return 1;
}

int execute_instruction(CPU *cpu, Instruction *instr) {
    if (!cpu || !instr) return 0;

    void *src = NULL;
    void *dest = NULL;

    if (instr->operand1) {
        dest = resolve_addressing(cpu, instr->operand1);
        if (!dest && strcmp(instr->mnemonic, "JMP") != 0 &&
            strcmp(instr->mnemonic, "JZ") != 0 && strcmp(instr->mnemonic, "JNZ") != 0) {
            return 0;
        }
    }

    if (instr->operand2) {
        src = resolve_addressing(cpu, instr->operand2);
        if (!src && strcmp(instr->mnemonic, "CMP") != 0) {
            return 0;
        }
    }

    return handle_instruction(cpu, instr, src, dest);
}

Instruction* fetch_next_instruction(CPU *cpu) {
    if (!cpu) return NULL;

    int* ip = (int*)hashmap_get(cpu->context, "IP");
    if (!ip) return NULL;

    Segment *cs_seg = (Segment *)hashmap_get(cpu->memory_handler->allocated, "CS");
    if (!cs_seg) return NULL;

    if (*ip < 0 || *ip >= cs_seg->size) return NULL;

    Instruction *instr = (Instruction *)load(cpu->memory_handler, "CS", *ip);
    if (!instr) return NULL;

    if (strcmp(instr->mnemonic, "JMP") != 0 &&
        strcmp(instr->mnemonic, "JZ") != 0 &&
        strcmp(instr->mnemonic, "JNZ") != 0) {
        (*ip)++;
    }

    return instr;
}

int run_program(CPU *cpu) {
    if (!cpu) return 0;

    printf("=== Initial CPU State ===\n");
    print_data_segment(cpu);
    print_registers(cpu);

    char input;
    while (1) {
        printf("\nPress Enter to execute next instruction or 'q' to quit...");
        input = getchar();
        if (input == 'q') break;

        Instruction *instr = fetch_next_instruction(cpu);
        if (!instr) {
            printf("No more instructions to execute or error occurred.\n");
            break;
        }

        printf("\nExecuting: %s", instr->mnemonic);
        if (instr->operand1) printf(" %s", instr->operand1);
        if (instr->operand2) printf(", %s", instr->operand2);
        printf("\n");

        if (!execute_instruction(cpu, instr)) {
            printf("Error executing instruction.\n");
            break;
        }

        print_registers(cpu);
    }

    printf("\n=== Final CPU State ===\n");
    print_data_segment(cpu);
    print_registers(cpu);

    return 1;
}


void print_registers(CPU *cpu) {
    if (!cpu) return;

    printf("Registers:\n");
    const char* reg_names[] = {"AX", "BX", "CX", "DX", "IP", "ZF", "SF", "SP", "BP"};
    for (int i = 0; i < 9; i++) {
        int* val = (int*)hashmap_get(cpu->context, reg_names[i]);
        if (val) {
            printf("%s: %d\n", reg_names[i], *val);
        }
    }
}


int push_value(CPU *cpu, int value) {
    if (!cpu) return -1;

    int* sp = (int*)hashmap_get(cpu->context, "SP");
    if (!sp) return -1;

    if (*sp < 0) {
        printf("Stack overflow!\n");
        return -1;
    }

    int* val_ptr = (int*)malloc(sizeof(int));
    if (!val_ptr) return -1;
    *val_ptr = value;

    store(cpu->memory_handler, "SS", *sp, val_ptr);

    (*sp)--;

    return 0;
}

int pop_value(CPU *cpu, int *dest) {
    if (!cpu || !dest) return -1;

    int* sp = (int*)hashmap_get(cpu->context, "SP");
    int* bp = (int*)hashmap_get(cpu->context, "BP");
    if (!sp || !bp) return -1;

    if (*sp >= *bp) {
        printf("Stack underflow!\n");
        return -1;
    }

    (*sp)++;

    int* val_ptr = (int*)load(cpu->memory_handler, "SS", *sp);
    if (!val_ptr) return -1;

    *dest = *val_ptr;
    free(val_ptr);

    return 0;
}