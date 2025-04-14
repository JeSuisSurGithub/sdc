#include "parser.h"
#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    CPU* cpu = cpu_init(1024);

    printf("CPU initialisé avec succès !\n");

    ParserResult* res = parse("exemple.asm");

    allocate_variables(cpu, res->data_instructions , res->data_count);

    printf("Segment DS alloué avec succès !\n");


    int* x = (int*)load(cpu->memory_handler, "DS", 0);
    int* y = (int*)load(cpu->memory_handler, "DS", 1);
    printf("Valeurs chargées : x = %d, y = %d\n", *x, *y);

    for (int i = 0; i < 4; i++) {
        int* arri = (int*)load(cpu->memory_handler, "DS", 2 + i);
        printf("arr[%d] = %d\n", i, *arri);
    }

    int* new_x = malloc(sizeof(int)); (*new_x) = -127;
    int* new_y = malloc(sizeof(int)); (*new_y) = 128;
    store(cpu->memory_handler, "DS", 0, new_x);
    store(cpu->memory_handler, "DS", 1, new_y);

    int* new_arr0 = malloc(sizeof(int)); (*new_arr0) = -32767;
    int* new_arr1 = malloc(sizeof(int)); (*new_arr1) = 32768;
    int* new_arr2 = malloc(sizeof(int)); (*new_arr2) = -2147483647;
    int* new_arr3 = malloc(sizeof(int)); (*new_arr3) = 2147483647;

    store(cpu->memory_handler, "DS", 2, new_arr0);
    store(cpu->memory_handler, "DS", 3, new_arr1);
    store(cpu->memory_handler, "DS", 4, new_arr2);
    store(cpu->memory_handler, "DS", 5, new_arr3);

    printf("Données stockées avec succès dans DS.\n");

    print_data_segment(cpu);

    cpu_destroy(cpu);
    free_parser_result(res);
    printf("CPU détruit et mémoire libérée.\n");

    return 0;
}

