#ifndef HASH_H
#define HASH_H

#define TABLE_SIZE 128
#define TOMBSTONE ((void*)-1)
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
	char* mnemonic ; // Instruction mnemonic (ou nom de variable pour .DATA)
	char* operand1 ; // Premier operande (ou type pour .DATA)
	char* operand2 ; // Second operande (ou initialisation pour .DATA)
} Instruction ;

typedef struct {
	Instruction ** data_instructions ; // Tableau d’instructions .DATA
	int data_count ; // Nombre d’instructions .DATA
	Instruction ** code_instructions ; // Tableau d’instructions .CODE
	int code_count ; // Nombre d’instructions .CODE
	HashMap* labels ; // labels -> indices dans code_instructions
	HashMap* memory_locations ; // noms de variables -> adresse memoire
} ParserResult;


Instruction* parse_data_instruction(const char* line, HashMap* memory_locations);
Instruction* parse_code_instruction(const char* line, HashMap* labels, int code_count);
ParserResult* parse(const char* filename);
void free_parser_result(ParserResult* result); 

int hashmap_remove(HashMap *map, const char *key);
void hashmap_destroy(HashMap *map);

#endif /* HASH_H */
