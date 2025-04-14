#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    CPU* cpu = setup_test_environment();

    int* nnn = resolve_addressing(cpu, "999");
    int* hhh = resolve_addressing(cpu, "888");
    int* bx = resolve_addressing(cpu, "BX");
    int* cx = resolve_addressing(cpu, "CX");
    int* ds0 = resolve_addressing(cpu, "[0]");
    int* ds9 = resolve_addressing(cpu, "[9]");
    int* dsax = resolve_addressing(cpu, "[AX]");
    int* dsdx = resolve_addressing(cpu, "[DX]");

    print_data_segment(cpu);
    printf("Avant:\n");
    printf("\tnnn: %i\n", *nnn);
    printf("\thhh: %i\n", *hhh);
    printf("\tbx: %i\n", *bx);
    printf("\tcx: %i\n", *cx);
    printf("\tds0: %i\n", *ds0);
    printf("\tds9: %i\n", *ds9);
    printf("\tdsax: %i\n", *dsax);
    printf("\tdsdx: %i\n", *dsdx);

    handle_MOV(cpu, nnn, cx);
    handle_MOV(cpu, bx, hhh);

    handle_MOV(cpu, ds0, dsax);
    handle_MOV(cpu, dsdx, ds9);

    print_data_segment(cpu);
    printf("Après:\n");
    printf("\tnnn: %i\n", *nnn);
    printf("\thhh: %i\n", *hhh);
    printf("\tbx: %i\n", *bx);
    printf("\tcx: %i\n", *cx);
    printf("\tds0: %i\n", *ds0);
    printf("\tds9: %i\n", *ds9);
    printf("\tdsax: %i\n", *dsax);
    printf("\tdsdx: %i\n", *dsdx);

    cpu_destroy(cpu);
    return EXIT_SUCCESS;
}

