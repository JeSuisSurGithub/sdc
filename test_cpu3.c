#include "cpu.h"
#include "parser.h"

int main()
{
    ParserResult* res = parse("exemple.asm");
    resolve_constants(res);

    CPU *cpu = cpu_init(1024);
    allocate_variables(cpu, res->data_instructions, res->data_count);
    allocate_code_segment(cpu, res->code_instructions, res->code_count);

    run_program(cpu);

    free_parser_result(res);
    cpu_destroy(cpu);
}