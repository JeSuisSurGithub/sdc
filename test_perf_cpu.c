#include "cpu.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

int run_program(CPU* cpu)
{
    if (cpu == NULL) return -1;

    puts("=== Initial CPU State ===\n");
    print_data_segment(cpu);
    print_registers(cpu);

    char input = 0;
    while (1) {
        Instruction *instr = fetch_next_instruction(cpu);
        if (instr == NULL) {
            puts("No more instructions to execute");
            break;
        }

        if (execute_instruction(cpu, instr) < 0) {
            puts("run_program(): error executing instruction");
            break;
        }
    }

    puts("\n=== Final CPU State ===");
    print_data_segment(cpu);
    print_registers(cpu);

    return 0;
}

int main(void)
{
    time_t start = clock();
    ParserResult* res = parse("perf_boucle.asm");
    if (resolve_constants(res) < 0) return -1;

    CPU* cpu = cpu_init(1024);

    allocate_variables(cpu, res->data_instructions, res->data_count);
    allocate_code_segment(cpu, res->code_instructions, res->code_count);

    run_program(cpu);

    free_parser_result(res);
    cpu_destroy(cpu);
    time_t end = clock();
    printf("execution time: %f", (float)(end-start) / CLOCKS_PER_SEC);
    return 0;
}