#include "cpu.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int run_program(CPU* cpu)
{
    if (cpu == NULL) return -1;

    puts("=== Etat initial du CPU ===\n");
    print_data_segment(cpu);
    print_registers(cpu);

    char input = 0;
    while (1) {
        puts("Appuyez sur entrée pour exécuter la prochain instruction ou q pour quitter...");
        input = getchar();
        if (input == 'q') break;

        Instruction *instr = fetch_next_instruction(cpu);
        if (instr == NULL) {
            puts("Aucune instruction à exécuter.");
            break;
        }

        printf("En cours d'exécution de: %s", instr->mnemonic);
        if (instr->operand1 && strcmp(instr->operand1, "") != 0) printf(" %s", instr->operand1);
        if (instr->operand2 && strcmp(instr->operand2, "") != 0) printf(", %s", instr->operand2);
        putchar('\n');

        if (execute_instruction(cpu, instr) < 0) {
            puts("run_program(): error executing instruction");
            break;
        }

        print_registers(cpu);
    }

    puts("\n=== Etat Final du CPU ===");
    print_data_segment(cpu);
    print_registers(cpu);

    return 0;
}


int main(int argc, char* argv[])
{
    puts("---- Projet SDC: Simulateur De CPU ----");
    puts("---- LU2IN006: Structures de Données ----");
    puts("---- de Charafeddine EL BOUHALI et Marc-Antoine XIA (TD Groupe 10) ----");

    if (argc != 3) {
        printf("Usage: %s [FICHIER_ASM] [TAILLE_MEMOIRE]", argv[0]);
        return EXIT_FAILURE;
    }

    char* f_asm = argv[1];
    ParserResult* res = parse(f_asm);
    if (res == NULL) {
        printf("L'analyse du fichier '%s' à échoué", f_asm);
        return EXIT_FAILURE;
    }

    int code = resolve_constants(res);
    if (code < 0) {
        printf("Le pré-traitement du fichier assembleur à échoué");
        return EXIT_FAILURE;
    }

    int mem_size = atoi(argv[2]);
    if (mem_size < 256) {
        printf("'%i' n'est pas une quantité de mémoire valable (256 minimum)", mem_size);
        return EXIT_FAILURE;
    }
    CPU* cpu = cpu_init(mem_size);
    if (cpu == NULL) {
        printf("La création du CPU à échouée");
        return EXIT_FAILURE;
    }

    printf("Allocation des variables...\n");
    allocate_variables(cpu, res->data_instructions, res->data_count);
    printf("Allocation des variables: terminée\n");

    printf("Allocation de segment de code...\n");
    allocate_code_segment(cpu, res->code_instructions, res->code_count);
    printf("Allocation de segment de code: terminée\n");

    run_program(cpu);

    printf("Terminaison... Libération des ressources...");

    free_parser_result(res);
    cpu_destroy(cpu);

    printf("Au revoir!");
    return EXIT_SUCCESS;
}