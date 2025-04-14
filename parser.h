#ifndef PARSER_H
#define PARSER_H

#include "hash.h"

#define DEFAULT_CODE_COUNT 10
#define DEFAULT_DATA_COUNT 10

#define MAX_NAME_LEN 40
#define MAX_TYPE_LEN 12
#define MAX_VALUE_LEN 24

#define MAX_LABEL_LEN 80
#define MAX_INS_LEN 12
#define MAX_OP1_LEN 40
#define MAX_OP2_LEN 40

#define MAX_LINE_LEN 180

typedef struct Instruction {
	char* mnemonic;
	char* operand1;
	char* operand2;
} Instruction ;

typedef struct ParserResult {
	Instruction** data_instructions;
	int data_count;
	Instruction** code_instructions;
	int code_count;
	HashMap* labels;
	HashMap* memory_locations;
} ParserResult;

Instruction* parse_data_instruction(const char* line, HashMap* memory_locations);
Instruction* parse_code_instruction(const char* line, HashMap* labels, int code_count);
ParserResult* parse(const char* filename);
void free_parser_result(ParserResult* result);

char* trim(char* str);
int search_and_replace(char** str, HashMap* values);
int resolve_constants(ParserResult *result);

#endif /* PARSER_H */
