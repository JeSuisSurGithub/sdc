#include "cpu.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main() {
    CPU *cpu = cpu_init(1024);
    if (!cpu) {
        fprintf(stderr, "Erreur d'initialisation du CPU\n");
        return 1;
    }

    printf("CPU initialisé avec succès !\n");

    ParserResult* parser = parse("exemple.asm");

    allocate_variables(cpu, parser->data_instructions , parser->data_count);

    printf("Segment DS alloué avec succès !\n");


    // int x = 3, y = 4, z = 5;
    // store(cpu->memory_handler, "DS", 0, &x);
    // store(cpu->memory_handler, "DS", 1, &y);
    // store(cpu->memory_handler, "DS", 2, &z);
// 
    // int arr[4] = {10, 20, 30, 40};
    // for (int i = 0; i < 4; i++) {
        // store(cpu->memory_handler, "DS", 3 + i, &arr[i]);
    // }
    // printf("Données stockées avec succès dans DS.\n");

    // print_data_segment(cpu);

    // int *loaded_x = (int*) load(cpu->memory_handler, "DS", 0);
    // int *loaded_y = (int*) load(cpu->memory_handler, "DS", 1);
    // int *loaded_z = (int*) load(cpu->memory_handler, "DS", 2);
    // printf("Valeurs chargées : x = %d, y = %d, z = %d\n", *loaded_x, *loaded_y, *loaded_z);

    // for (int i = 0; i < 4; i++) {
    //     int *loaded_arr = (int*) load(cpu->memory_handler, "DS", 3 + i);
    //     printf("arr[%d] = %d\n", i, *loaded_arr);
    // }


    cpu_destroy(cpu);
    free_parser_result(parser);
    printf("CPU détruit et mémoire libérée.\n");

    return EXIT_SUCCESS;
}

