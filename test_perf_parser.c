#include "parser.h"

#include <stdio.h>
#include <time.h>

#define ITERATIONS 10000

int main(void)
{
    time_t start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        ParserResult* result = parse("perf_compile.asm");
        free_parser_result(result);
    }
    time_t end = clock();
    printf("%ix compile time: %f", ITERATIONS, (float)(end-start) / CLOCKS_PER_SEC);
    return 0;
}