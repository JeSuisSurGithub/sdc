#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "hash.h"
#include "parser.h"

int main() {
    CPU* cpu = cpu_init(1024);
    if (!cpu) {
        printf("Erreur d'initialisation du CPU\n");
        return 1;
    }

    int* ax = hashmap_get(cpu->context, "AX");
    int* bx = hashmap_get(cpu->context, "BX");
    int* zf = hashmap_get(cpu->context, "ZF");
    int* es = hashmap_get(cpu->context, "ES");

    *ax = 6;
    *bx = 0;

    Instruction alloc_instr = { .mnemonic = "ALLOC", .operand1 = NULL, .operand2 = NULL };
    handle_instruction(cpu, &alloc_instr, NULL, NULL);

    if (*zf == 1) {
        printf("Échec de l'allocation du segment ES\n");
        return 1;
    } else {
        printf("Allocation réussie du segment ES à l'adresse %d\n", *es);
    }

    // MOV [ES:AX], 42
    int val = 42;
    int* cx = hashmap_get(cpu->context, "CX");
    *cx = 2;  // Position où on veut écrire

    char es_operand[] = "[ES:CX]";
    void* dest = resolve_addressing(cpu, es_operand);
    if (!dest) {
        printf("Erreur dans l'adressage segmenté\n");
        return 1;
    }

    handle_MOV(cpu, &val, dest);
    printf("Valeur 42 écrite à ES + CX\n");

    int read_val = *(int*)resolve_addressing(cpu, "[ES:CX]");
    printf("Valeur lue depuis [ES:CX] = %d\n", read_val);

    Instruction free_instr = { .mnemonic = "FREE", .operand1 = NULL, .operand2 = NULL };
    handle_instruction(cpu, &free_instr, NULL, NULL);
    printf("Segment ES libéré\n");

    cpu_destroy(cpu);
    printf("(Ici il y a deux messages d'avertissements car on n'a pas alloué CS ni DS)\n");
    return 0;
}

