void *segment_override_addressing(CPU* cpu, const char* operand) {
    regex_t regex;
    regcomp(&regex, "^\\[([A-Z]{2}):([A-Z]{2})\\]$", REG_EXTENDED);
    regmatch_t matches[3];
    
    if (regexec(&regex, operand, 3, matches, 0) == 0) {
        char segment[3], reg[3];
        strncpy(segment, operand + matches[1].rm_so, 2);
        segment[2] = '\0';
        strncpy(reg, operand + matches[2].rm_so, 2);
        reg[2] = '\0';

        int *reg_val = hashmap_get(cpu->context, reg);
        if (!reg_val) return NULL;

        Segment* seg = hashmap_get(cpu->memory_handler->allocated, segment);
        if (!seg || *reg_val < 0 || *reg_val >= seg->size) return NULL;

        void **memory = cpu->memory_handler->memory;
        int addr = seg->start + *reg_val;

        regfree(&regex);
        return memory[addr];
    }

    regfree(&regex);
    return NULL;
}

void *resolve_addressing(CPU *cpu, const char *operand) {
    void *res = NULL;
    if ((res = segment_override_addressing(cpu, operand))) return res;
    if ((res = register_indirect_addressing(cpu, operand))) return res;
    if ((res = memory_direct_addressing(cpu, operand))) return res;
    if ((res = register_addressing(cpu, operand))) return res;
    if ((res = immediate_addressing(cpu, operand))) return res;
    return NULL;
}

int find_free_address_strategy(MemoryHandler *handler, int size, int strategy) {
    Segment *curr = handler->free_list;
    int best_addr = -1, best_fit_size = __INT_MAX__;
    int worst_fit_size = -1;

    while (curr) {
        if (curr->size >= size) {
            if (strategy == 0) return curr->start;
            else if (strategy == 1 && curr->size < best_fit_size) {
                best_fit_size = curr->size;
                best_addr = curr->start;
            } else if (strategy == 2 && curr->size > worst_fit_size) {
                worst_fit_size = curr->size;
                best_addr = curr->start;
            }
        }
        curr = curr->next;
    }
    return best_addr;
}

// Manque la question 8.4

int alloc_es_segment(CPU *cpu) {
    int *ax = hashmap_get(cpu->context, "AX");
    int *bx = hashmap_get(cpu->context, "BX");
    int *zf = hashmap_get(cpu->context, "ZF");
    if (!ax || !bx || !zf) return -1;

    int addr = find_free_address_strategy(cpu->memory_handler, *ax, *bx);
    if (addr == -1) {
        *zf = 1;
        return -1;
    }

    if (create_segment(cpu->memory_handler, "ES", addr, *ax) == -1) {
        *zf = 1;
        return -1;
    }

    for (int i = 0; i < *ax; i++) {
        int *val = malloc(sizeof(int));
        *val = 0;
        store(cpu->memory_handler, "ES", i, val);
    }

    hashmap_put(cpu->context, "ES", (void*)(long)addr);
    *zf = 0;
    return 0;
}

int free_es_segment(CPU *cpu) {
    int *es = hashmap_get(cpu->context, "ES");
    if (!es || *es == -1) return -1;

    Segment *seg = hashmap_get(cpu->memory_handler->allocated, "ES");
    if (!seg) return -1;

    for (int i = 0; i < seg->size; i++) {
        void **memory = cpu->memory_handler->memory;
        int addr = seg->start + i;
        if (memory[addr]) {
            free(memory[addr]);
            memory[addr] = NULL;
        }
    }

    hashmap_remove(cpu->memory_handler->allocated, "ES");
    Segment *new_seg = malloc(sizeof(Segment));
    new_seg->start = seg->start;
    new_seg->size = seg->size;
    new_seg->next = cpu->memory_handler->free_list;
    cpu->memory_handler->free_list = new_seg;

    hashmap_put(cpu->context, "ES", (void*)(long)(-1));
    return 0;
}

int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest) {
    if (strcmp(instr->mnemonic, "ALLOC") == 0) return alloc_es_segment(cpu);
    if (strcmp(instr->mnemonic, "FREE") == 0) return free_es_segment(cpu);

    // Faut rajouter aussi les autres instructions classiques (MOV, ADD, etc.)
    return 0;
}

