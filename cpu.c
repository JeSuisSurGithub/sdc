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
    hashmap_insert(cpu->context, "SP", sp);
    hashmap_insert(cpu->context, "BP", bp);

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

    free(hashmap_get(cpu->context, "BP"));
    free(hashmap_get(cpu->context, "SP"));

    Segment* DS = hashmap_get(cpu->memory_handler->allocated, "DS");
	for (int i = 0; i < (DS->size / sizeof(int)); i++)
    {
    	if (cpu->memory_handler->memory[i] != NULL) {
    		free(cpu->memory_handler->memory[i]);
    	}
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
        if (handler->memory[seg->start + pos] != NULL) {
            free(handler->memory[seg->start + pos]);
            handler->memory[seg->start + pos] = NULL;
        }
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
    unsigned int ds_size = 0;
    for (int i = 0; i < data_count; i++) {
        char* arr = data_instructions[i]->operand2;

        unsigned int ins_size = 0;
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
    if (create_segment(cpu->memory_handler, "DS", 0, ds_size * sizeof(int)) < 0) {
        puts("allocate_variables(): create_segment failed");
    }
}

void print_data_segment(CPU* cpu)
{
    if (cpu != NULL) {
		Segment* DS = hashmap_get(cpu->memory_handler->allocated, "DS");
   		for (int i = DS->start; i < (DS->start + DS->size); i++) {
    		if (cpu->memory_handler->memory[i] != NULL) {
    			printf("DS[%i] = %i | ", i, *(int*)(cpu->memory_handler->memory[i]));
    		}
    	}
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
        return cpu->memory_handler->memory[addr];
    }
    return NULL;
}

void* register_indirect_addressing(CPU* cpu, const char* operand)
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
    if (cpu == NULL) {
        puts("setup_test_environment(): cpu_init failed");
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
        if (create_segment(cpu->memory_handler, "DS", 0, 10 * sizeof(int)) < 0) {
            puts("setup_test_environment(): create_segment failed");
        }

        for (int i = 0; i < 10; i++) {
            int* value = (int*)malloc(sizeof(int));
            *value = i * 10 + 5;
            store(cpu->memory_handler, "DS", i, value);
        }
    }

    puts("setup_test_environment(): test environnment initialized");
    return cpu;
}

void* resolve_addressing(CPU* cpu, const char* operand)
{
    void* imm = immediate_addressing(cpu, operand);
    if (imm != NULL) return imm;

    void* reg = register_addressing(cpu, operand);
    if (reg != NULL) return reg;

    void* direct = memory_direct_addressing(cpu, operand);
    if (direct != NULL) return direct;

    void* indirect = register_indirect_addressing(cpu, operand);
    if (indirect != NULL) return indirect;

    puts("resolve_addressing(): operand doesn't match with any mode");
    return NULL;
}

void allocate_code_segment(CPU* cpu, Instruction** code_instructions, int code_count)
{
    if ((cpu == NULL) || (code_instructions == NULL) || code_count <= 0) {
        return;
    }

    Segment* DS = hashmap_get(cpu->memory_handler->allocated, "DS");

    if (create_segment(cpu->memory_handler, "CS", (DS->start + DS->size), code_count) < 0) {
        puts("allocate_code_segment(): create_segment failed");
    }

    for (int i = 0; i < code_count; i++) {
        store(cpu->memory_handler, "CS", i, code_instructions[i]);
    }

    int* ip = (int*)hashmap_get(cpu->context, "IP");
    *ip = 0;
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

int run_program(CPU* cpu)
{
    if (cpu == NULL) return -1;

    puts("=== Initial CPU State ===\n");
    print_data_segment(cpu);
    print_registers(cpu);

    char input = 0;
    while (1) {
        puts("Press Enter to execute next instruction or 'q' to quit...");
        input = getchar();
        if (input == 'q') break;

        Instruction *instr = fetch_next_instruction(cpu);
        if (instr == NULL) {
            puts("No more instructions to execute");
            break;
        }

        printf("Executing: %s", instr->mnemonic);
        if (instr->operand1) printf(" %s", instr->operand1);
        if (instr->operand2) printf(", %s", instr->operand2);
        putchar('\n');

        if (execute_instruction(cpu, instr) < 0) {
            puts("run_program(): error executing instruction");
            break;
        }

        print_registers(cpu);
    }

    puts("\n=== Final CPU State ===");
    print_data_segment(cpu);
    print_registers(cpu);

    return 0;
}


void print_registers(CPU* cpu)
{
    if (cpu == NULL) return;

    printf("Registers:\n");
    const char* reg_names[] = {"AX", "BX", "CX", "DX", "IP", "ZF", "SF", "SP", "BP"};
    for (int i = 0; i < 9; i++) {
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

    int* sp = (int*)hashmap_get(cpu->context, "SP");
    if (sp == NULL) return -1;

    if (*sp < 0) {
        printf("push_value(): stack overflow!\n");
        return -1;
    }

    int* val_ptr = (int*)malloc(sizeof(int));
    if (val_ptr == NULL) return -1;
    *val_ptr = value;

    store(cpu->memory_handler, "SS", *sp, val_ptr);

    (*sp)--;

    return 0;
}

int pop_value(CPU* cpu, int* dest) {
    if (cpu == NULL || dest == NULL) return -1;

    int* sp = (int*)hashmap_get(cpu->context, "SP");
    int* bp = (int*)hashmap_get(cpu->context, "BP");
    if (sp == NULL || bp == NULL) return -1;

    if (*sp >= *bp) {
        printf("pop_value(): stack underflow!\n");
        return -1;
    }

    (*sp)++;

    int* val_ptr = (int*)load(cpu->memory_handler, "SS", *sp);
    if (val_ptr == NULL) return -1;

    *dest = *val_ptr;
    free(val_ptr);

    return 0;
}