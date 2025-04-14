#include "parser.h"

#include <stdio.h>

int main(void)
{
    ParserResult* result = parse("exemple.asm");

    if (!result) {
        printf("Erreur lors de l'analyse du fichier\n");
        return 1;
    }

    printf("Instructions .data (%d) :\n", result->data_count);
    for (int i = 0; i < result->data_count; i++) {
        printf("'%s '%s' '%s'\n",
               result->data_instructions[i]->mnemonic,
               result->data_instructions[i]->operand1,
               result->data_instructions[i]->operand2);
    }

    printf("Instructions .code (%d) :\n", result->code_count);
    for (int i = 0; i < result->code_count; i++) {
        printf("'%s' '%s' '%s'\n",
               result->code_instructions[i]->mnemonic,
               result->code_instructions[i]->operand1,
               result->code_instructions[i]->operand2);
    }

    printf("Labels (%d) :\n", result->labels->size);
    for (int i = 0; i < TABLE_SIZE; i++) {
    	char* label = result->labels->table[i].key;
        if (label) {
        	int* addr = (int*)hashmap_get(result->labels, label);
            printf("'%s' -> %d\n", label, *addr);
        }
    }

    printf("Emplacements mémoire (%d) :\n", result->memory_locations->size);
    for (int i = 0; i < TABLE_SIZE; i++) {
    	char* mnemonic = result->memory_locations->table[i].key;
        if (mnemonic) {
            int* addr = hashmap_get(result->memory_locations, mnemonic);
            printf("'%s' -> %d\n", mnemonic, *addr);
        }
    }

    free_parser_result(result);

    return 0;
}

