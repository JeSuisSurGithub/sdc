#include "parser.h"
#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

Instruction* parse_data_instruction(const char *line, HashMap *memory_locations) {
    Instruction *inst = (Instruction*)malloc(sizeof(Instruction));
    if (!inst) return NULL;

    char nom[100], type[10], valeur[256];

    // Format : nom type valeurs
    int n = sscanf(line, "%s %s %[^\n]", nom, type, valeur);
    if (n != 3) {
        puts("parse_data_instruction: invalid format");
        free(inst);
        return NULL;
    }

    inst->mnemonic = strdup(nom);
    inst->operand1 = strdup(type);
    inst->operand2 = strdup(valeur);

    static int adresse_suiv = 0;
    int nb_elem = 1;
    for (int i = 0; valeur[i]; i++) {
        if (valeur[i] == ',') nb_elem++;
    }

    int* adresse_suiv_mem = malloc(sizeof(int));
    if (adresse_suiv_mem == NULL) {
        puts("parse_data_instruction: malloc failed");
        free(inst);
        return NULL;
    }
    (*adresse_suiv_mem) = adresse_suiv;

    ;
    if (hashmap_insert(memory_locations, inst->mnemonic, adresse_suiv_mem) < 0) {
        puts("parse_data_instruction: hashmap_insert failed");
        free(inst);
        return NULL;
    }
    adresse_suiv += nb_elem;

    return inst;
}

Instruction* parse_code_instruction(const char* line, HashMap* labels, int code_count) {
    Instruction* inst = (Instruction*)malloc(sizeof(Instruction));
    if (!inst) {
        puts("parse_code_instruction: malloc failed");
        return NULL;
    }

    char label[100] = "", mnemonic[20] = "", op1[100] = "", op2[100] = "";
    int i = 0;

    // Détecter s'il y a un label
    while (line[i] != '\0' && line[i] != ':' && i < 99) {
        label[i] = line[i];
        i++;
    }

    if (line[i] == ':') {
        label[i] = '\0';
        i++; // Sauter le ":"

        int* code_count_mem = malloc(sizeof(int));
        if (code_count_mem == NULL) {
            puts("parse_code_instruction: malloc failed");
            free(inst);
            return NULL;
        }

        (*code_count_mem) = code_count;

        if (hashmap_insert(labels, label, code_count_mem) < 0) {
            puts("parse_code_instruction: hashmap_insert failed");
            free(inst);
            free(code_count_mem);
            return NULL;
        }
    } else {
        // Pas de label, réinitialisez l'index à 0
        i = 0;
    }

    while (line[i] == ' ') i++;

    // Lire le reste de la ligne (instruction)
    int n = sscanf(line + i, "%s %[^,],%[^\n]", mnemonic, op1, op2);

    if (n < 1) {
        puts("parse_code_instruction: invalid format");
        free(inst);
        return NULL; // Ligne ignorée
    }

    inst->mnemonic = strdup(mnemonic);

    // Si l'instruction n'a pas d'opérandes, on remplit les opérandes avec des chaînes vides
    inst->operand1 = (n >= 2 && op1[0] != '\0') ? strdup(op1) : strdup("");
    inst->operand2 = (n == 3 && op2[0] != '\0') ? strdup(op2) : strdup("");

    return inst;
}

ParserResult* parse(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        puts("Erreur lors de l'ouverture du fichier");
        return NULL;
    }

    // Verifications à faire
    ParserResult* result = malloc(sizeof(ParserResult));
    result->data_instructions = malloc(sizeof(Instruction*) * MAX_DATA_COUNT);
    result->code_instructions = malloc(sizeof(Instruction*) * MAX_CODE_COUNT);
    result->labels = hashmap_create();
    result->memory_locations = hashmap_create();
    result->data_count = 0;
    result->code_count = 0;
    int data_capacite = MAX_DATA_COUNT, code_capacite = MAX_CODE_COUNT;

    char line[MAX_LINE_LENGTH];
    int in_data_section = 0, in_code_section = 0;

    while (fgets(line, sizeof(line), file)) {
        // Supprimer le saut de ligne
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Vérifiez si la ligne est vide
        if (strlen(line) == 0) {
            continue; // Ignore les lignes vides
        }

        // Identifier les sections
        if (strcmp(line, ".DATA") == 0) {
            in_data_section = 1;
            in_code_section = 0;
            continue;
        } else if (strcmp(line, ".CODE") == 0) {
            in_data_section = 0;
            in_code_section = 1;
            continue;
        }

        if (in_data_section) {
            if (result->data_count >= data_capacite) {
                data_capacite *= 2;
                result->data_instructions = realloc(result->data_instructions, sizeof(Instruction*) * data_capacite);
                if (result->data_instructions == NULL) {
                    puts("Erreur de réallocation de mémoire");
                    fclose(file);
                    return NULL;
                }
            }
            Instruction* instruction = parse_data_instruction(line, result->memory_locations);
            if (instruction != NULL) {
                result->data_instructions[result->data_count++] = instruction;
            } else {
                fprintf(stderr, "Erreur lors de l'analyse de la ligne : %s\n", line);
            }
        } else if (in_code_section) {
            if (result->code_count >= code_capacite) {
                code_capacite *= 2;
                result->code_instructions = realloc(result->code_instructions, sizeof(Instruction*) * code_capacite);
                if (result->code_instructions == NULL) {
                    puts("Erreur de réallocation de mémoire");
                    fclose(file);
                    return NULL;
                }
            }
            Instruction* instruction = parse_code_instruction(line, result->labels, result->code_count);
            if (instruction != NULL) {
                result->code_instructions[result->code_count++] = instruction;
            } else {
                fprintf(stderr, "Erreur lors de l'analyse de la ligne : %s\n", line);
            }
        }
    }

    fclose(file);
    return result;
}

void free_parser_result(ParserResult* result)
{
    if (result)
    {
        for (int i = 0; i < result->data_count; i++)
        {
            free(result->data_instructions[i]->mnemonic);
            free(result->data_instructions[i]->operand1);
            free(result->data_instructions[i]->operand2);
            free(result->data_instructions[i]);
        }
        free(result->data_instructions);

        for (int i = 0; i < result->code_count; i++)
        {
            free(result->code_instructions[i]->mnemonic);
            free(result->code_instructions[i]->operand1);
            free(result->code_instructions[i]->operand2);
            free(result->code_instructions[i]);
        }
        free(result->code_instructions);

        for (int i = 0; i < TABLE_SIZE; i++)
        {
            if (result->labels->table[i].key != NULL) {
                free(result->labels->table[i].value);
            }
        }

        hashmap_destroy(result->labels);

        for (int i = 0; i < TABLE_SIZE; i++)
        {
            if (result->memory_locations->table[i].key != NULL) {
                free(result->memory_locations->table[i].value);
            }
        }

        hashmap_destroy(result->memory_locations);

        free(result);
    }
}

char* trim(char* str)
{
    while (*str == ' ' || *str == '\t' || * str == '\n' || * str == '\r') str++;
    char* end = str + strlen (str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
    {
        *end = '\0';
        end--;
    }
    return str;
}

int search_and_replace(char** str, HashMap* values)
{
    if (!str || !*str || !values) return 0;
    int replaced = 0;
    char* input = *str;
    printf("str: %s\n", input);
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (values->table[i].key != NULL && values->table[i].key != TOMBSTONE)
        {
            printf("\tvalues: %s %i\n", values->table[i].key, *(int*)values->table[i].value);
            char* key = values->table[i].key;
            int value = *(int*)values->table[i].value;

            char* substr = strstr(input, key);
            if (substr) {
                char replacement[64];
                snprintf(replacement, sizeof(replacement), "[%d]", value);

                int key_len = strlen(key);
                int repl_len = strlen(replacement);
                int remain_len = strlen(substr + key_len);

                char* new_str = (char*)malloc(strlen(input) - key_len + repl_len + 1);
                strncpy(new_str, input, substr - input);
                new_str[substr - input] = '\0';
                strcat(new_str, replacement);
                strcat(new_str, substr + key_len);

                free(input);
                *str = new_str;
                input = new_str;

                replaced = 1;
            }
        }
    }

    if (replaced) {
        char* trimmed = trim(input);
        if (trimmed != input) {
            memmove(input, trimmed, strlen(trimmed) + 1);
        }
    }

    return replaced;
}

int resolve_constants(ParserResult* result)
{
    if (!result) {
        return 0;
    }

    for (int i = 0; i < result->code_count; i++)
    {
        Instruction *instr = result->code_instructions[i];
        if (instr->operand1) {
            search_and_replace(&instr->operand1, result->memory_locations);
        }
        if (instr->operand2) {
            search_and_replace(&instr->operand2, result->memory_locations);
        }
    }

    for (int i = 0; i < result->code_count; i++)
    {
        Instruction *instr = result->code_instructions[i];
        if (instr->operand1) {
            search_and_replace(&instr->operand1, result->labels);
        }
        if (instr->operand2) {
            search_and_replace(&instr->operand2, result->labels);
        }
    }

    return 1;
}