#include "parser.h"

#include <stdio.h>

int main(void)
{
    ParserResult* result = parse("exemple.asm");

    if (!result) {
        printf("Erreur lors de l'analyse du fichier.\n");
        return 1;
    }

    printf("Nombre d'instructions .DATA : %d\n", result->data_count);
    for (int i = 0; i < result->data_count; i++) {
        printf("%s %s %s\n",
               result->data_instructions[i]->mnemonic,
               result->data_instructions[i]->operand1,
               result->data_instructions[i]->operand2);
    }

    printf("\nNombre d'instructions .CODE : %d\n", result->code_count);
    for (int i = 0; i < result->code_count; i++) {
        printf("%s %s %s\n",
               result->code_instructions[i]->mnemonic,
               result->code_instructions[i]->operand1,
               result->code_instructions[i]->operand2);
    }

    printf("\nLabels :\n");
    printf("result->codecount=%d\n", result->code_count);

    for (int i = 0; i < TABLE_SIZE; i++) {
    	char* label = result->labels->table[i].key;
        if (label != NULL) {
        	int* addr = hashmap_get(result->labels, label);
            printf("%s -> %d\n", label, *addr);
        }
    }

    for (int i = 0; i < result->code_count; i++) {
    	printf("i=%d\n", i);
    	printf("mnemonic=%s\n", result->code_instructions[i]->mnemonic);
        int* label = hashmap_get(result->labels, result->code_instructions[i]->mnemonic);
        if(label==NULL){
        	printf("Label est NULL\n");
        }
        if(label!=NULL){
        	printf("Label n'est pas NULL\n");
        }
        if (label) {
            printf("%s -> %d\n", result->code_instructions[i]->mnemonic, *label);
        }
    }

    printf("\nEmplacements mémoire :\n");
    for (int i = 0; i < result->data_count; i++) {
        void *address = hashmap_get(result->memory_locations, result->data_instructions[i]->mnemonic);
        if (address) {
            printf("%s -> %p\n", result->data_instructions[i]->mnemonic, address);
        }
    }

    free_parser_result(result);

    return 0;
}

