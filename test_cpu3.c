#include "cpu.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>

int run_program(CPU* cpu)
{
    if (cpu == NULL) return -1;

    puts("=== Initial CPU State ===\n");
    print_data_segment(cpu);
    print_registers(cpu);

    char input = 0;
    while (1) {
        puts("Press Enter to execute next instruction or 'q' to quit...");
        input = getchar();
        if (input == 'q') break;

        Instruction *instr = fetch_next_instruction(cpu);
        if (instr == NULL) {
            puts("No more instructions to execute");
            break;
        }

        printf("Executing: %s", instr->mnemonic);
        if (instr->operand1 && strcmp(instr->operand1, "") != 0) printf(" %s", instr->operand1);
        if (instr->operand2 && strcmp(instr->operand2, "") != 0) printf(", %s", instr->operand2);
        putchar('\n');

        if (execute_instruction(cpu, instr) < 0) {
            puts("run_program(): error executing instruction");
            break;
        }

        print_registers(cpu);
    }

    puts("\n=== Final CPU State ===");
    print_data_segment(cpu);
    print_registers(cpu);

    return 0;
}


int main(void)
{
    ParserResult* res = parse("exemple.asm");
    if (resolve_constants(res) < 0) return -1;

    CPU* cpu = cpu_init(1024);

    allocate_variables(cpu, res->data_instructions, res->data_count);
    allocate_code_segment(cpu, res->code_instructions, res->code_count);

    run_program(cpu);

    free_parser_result(res);
    cpu_destroy(cpu);
    return 0;
}