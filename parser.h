#ifndef PARSER_H
#define PARSER_H

#include "hash.h"

typedef struct {
	char* mnemonic;
	char* operand1;
	char* operand2;
} Instruction ;

typedef struct {
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

#endif /* PARSER_H */
