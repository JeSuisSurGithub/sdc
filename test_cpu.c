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

    print_data_segment(cpu);

    cpu_destroy(cpu);
    free_parser_result(res);
    printf("CPU détruit et mémoire libérée.\n");

    return 0;
}

