CPU *cpu_init(int memory_size) {
    int *sp = (int *)malloc(sizeof(int));
    int *bp = (int *)malloc(sizeof(int));
    *sp = *bp = 0;
    hashmap_insert(cpu->context, "SP", sp);
    hashmap_insert(cpu->context, "BP", bp);

    create_segment(cpu->memory_handler, "SS", 0, 128); 
    *bp = 127; 
    *sp = 127; 

    return cpu;
}

int push_value(CPU *cpu, int value) {
    if (!cpu) return -1;

    int *sp = (int *)hashmap_get(cpu->context, "SP");
    if (!sp) return -1;

    if (*sp < 0) {
        printf("Stack overflow!\n");
        return -1;
    }

    int *val_ptr = (int *)malloc(sizeof(int));
    if (!val_ptr) return -1;
    *val_ptr = value;

    store(cpu->memory_handler, "SS", *sp, val_ptr);

    (*sp)--;

    return 0;
}

int pop_value(CPU *cpu, int *dest) {
    if (!cpu || !dest) return -1;

    int *sp = (int *)hashmap_get(cpu->context, "SP");
    int *bp = (int *)hashmap_get(cpu->context, "BP");
    if (!sp || !bp) return -1;

    if (*sp >= *bp) {
        printf("Stack underflow!\n");
        return -1;
    }

    (*sp)++;

    int *val_ptr = (int *)load(cpu->memory_handler, "SS", *sp);
    if (!val_ptr) return -1;

    *dest = *val_ptr;
    free(val_ptr); 

    return 0;
}

int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest) {

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
        if (pop_value(cpu, &value) return 0;
        
        if (!dest) {
            int *ax = (int *)hashmap_get(cpu->context, "AX");
            if (!ax) return 0;
            *ax = value;
        } else {
            *(int *)dest = value;
        }
        return 1;
    }

    return 0;
}
