int resolve_constants(ParserResult *result) {
    if (!result) return 0;

    for (int i = 0; i < result->code_count; i++) {
        Instruction *instr = result->code_instructions[i];
        if (instr->operand1) {
            search_and_replace(&instr->operand1, result->memory_locations);
        }
        if (instr->operand2) {
            search_and_replace(&instr->operand2, result->memory_locations);
        }
    }

    for (int i = 0; i < result->code_count; i++) {
        Instruction *instr = result->code_instructions[i];
        if (instr->operand1) {
            search_and_replace(&instr->operand1, result->labels);
        }
        if (instr->operand2) {
            search_and_replace(&instr->operand2, result->labels);
        }
    }

    return 1;
}

CPU *cpu_init(int memory_size) {
    CPU *cpu = (CPU *)malloc(sizeof(CPU));
    if (!cpu) return NULL;

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

    int *ax = (int *)malloc(sizeof(int));
    int *bx = (int *)malloc(sizeof(int));
    int *cx = (int *)malloc(sizeof(int));
    int *dx = (int *)malloc(sizeof(int));
    *ax = *bx = *cx = *dx = 0;
    hashmap_insert(cpu->context, "AX", ax);
    hashmap_insert(cpu->context, "BX", bx);
    hashmap_insert(cpu->context, "CX", cx);
    hashmap_insert(cpu->context, "DX", dx);

    int *ip = (int *)malloc(sizeof(int));
    int *zf = (int *)malloc(sizeof(int));
    int *sf = (int *)malloc(sizeof(int));
    *ip = *zf = *sf = 0;
    hashmap_insert(cpu->context, "IP", ip);
    hashmap_insert(cpu->context, "ZF", zf);
    hashmap_insert(cpu->context, "SF", sf);

    cpu->constant_pool = hashmap_create();

    return cpu;
}

void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count) {
    if (!cpu || !code_instructions || code_count <= 0) return;

    create_segment(cpu->memory_handler, "CS", 0, code_count);

    for (int i = 0; i < code_count; i++) {
        store(cpu->memory_handler, "CS", i, code_instructions[i]);
    }

    int *ip = (int *)hashmap_get(cpu->context, "IP");
    *ip = 0;
}

int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest) {
    if (!cpu || !instr) return 0;

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
        int result = *(int *)dest - *(int *)src;
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

    int *ip = (int *)hashmap_get(cpu->context, "IP");
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


// Fonction pour afficher les registres (utilisée dans run_program)
void print_registers(CPU *cpu) {
    if (!cpu) return;

    printf("Registers:\n");
    const char *reg_names[] = {"AX", "BX", "CX", "DX", "IP", "ZF", "SF", "SP", "BP"};
    for (int i = 0; i < 9; i++) {
        int *val = (int *)hashmap_get(cpu->context, reg_names[i]);
        if (val) {
            printf("%s: %d\n", reg_names[i], *val);
        }
    }
}

