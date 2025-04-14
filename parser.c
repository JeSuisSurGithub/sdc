#include "parser.h"
#include "hash.h"
#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Instruction* parse_data_instruction(const char* line, HashMap* memory_locations)
{
    Instruction* inst = (Instruction*)malloc(sizeof(Instruction));
    if (inst == NULL) {
        puts("parse_data_instruction(): malloc failed");
        return NULL;
    }

    char nom[MAX_NAME_LEN] = {0};
    char type[MAX_TYPE_LEN] = {0};
    char valeur[MAX_VALUE_LEN] = {0};

    // Nom Type Valeur
    int n = sscanf(line, "%s %s %[^\n]", nom, type, valeur);
    if (n != 3) {
        puts("parse_data_instruction(): invalid format");
        free(inst);
        return NULL;
    }

    inst->mnemonic = strdup(nom);
    inst->operand1 = strdup(type);
    inst->operand2 = strdup(valeur);

    // adresse_suiv vit entre plusieurs appels successifs de cette fonctions
    static int adresse_suiv = 0;

    int* adresse_suiv_mem = malloc(sizeof(int));
    if (adresse_suiv_mem == NULL) {
        puts("parse_data_instruction(): malloc failed (2)");
        free(inst);
        return NULL;
    }
    (*adresse_suiv_mem) = adresse_suiv;

    // Compte des éléments
    int nb_elem = 1;
    for (int i = 0; valeur[i]; i++) {
        if (valeur[i] == ',') nb_elem++;
    }

    if (hashmap_insert(memory_locations, inst->mnemonic, adresse_suiv_mem) < 0) {
        puts("parse_data_instruction(): hashmap_insert failed");
        free(inst);
        return NULL;
    }
    adresse_suiv += nb_elem;

    return inst;
}

Instruction* parse_code_instruction(const char* line, HashMap* labels, int code_count)
{
    Instruction* inst = (Instruction*)malloc(sizeof(Instruction));
    if (inst == NULL) {
        puts("parse_code_instruction(): malloc failed");
        return NULL;
    }

    char label[MAX_LABEL_LEN] = {0};
    char mnemonic[MAX_INS_LEN] = {0};
    char op1[MAX_OP1_LEN] = {0};
    char op2[MAX_OP2_LEN] = {0};

    int i = 0;

    // Détecter s'il y a un label (strchr???)
    while (line[i] != '\0' && line[i] != ':' && i < (MAX_LABEL_LEN - 1)) {
        label[i] = line[i];
        i++;
    }

    // Si il y a
    if (line[i] == ':') {
        label[i] = '\0';

        int* code_count_mem = malloc(sizeof(int));
        if (code_count_mem == NULL) {
            puts("parse_code_instruction(): malloc failed (2)");
            free(inst);
            return NULL;
        }
        (*code_count_mem) = code_count;

        if (hashmap_insert(labels, label, code_count_mem) < 0) {
            puts("parse_code_instruction(): hashmap_insert failed");
            free(inst);
            free(code_count_mem);
            return NULL;
        }
        i++; // Sauter le ":"
    } else {
        // Pas de label, retour à l'index à 0
        i = 0;
    }

    while (line[i] == ' ') i++;

    // Lire le reste de la ligne (instruction)
    int n = sscanf(line + i, "%s %[^,],%[^\n]", mnemonic, op1, op2);

    if (n < 1) {
        puts("parse_code_instruction(): invalid format");
        free(inst);
        return NULL; // Ligne ignorée
    }

    // Si l'instruction n'a pas d'opérandes, on remplit les opérandes avec des chaînes vides
    inst->mnemonic = strdup(mnemonic);
    inst->operand1 = (n >= 2 && op1[0] != '\0') ? strdup(op1) : strdup("");
    inst->operand2 = (n == 3 && op2[0] != '\0') ? strdup(op2) : strdup("");

    return inst;
}

ParserResult* parse(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        puts("parse(): cannot open file");
        return NULL;
    }

    // Verifications à faire
    ParserResult* result = malloc(sizeof(ParserResult));
    if (result == NULL) {
        puts("parse(): malloc failed");
        return NULL;
    }
    result->data_instructions = malloc(sizeof(Instruction*) * DEFAULT_DATA_COUNT);
    if (result->data_instructions == NULL) {
        puts("parse(): malloc failed (2)");
        free(result);
        return NULL;
    }
    result->code_instructions = malloc(sizeof(Instruction*) * DEFAULT_CODE_COUNT);
    if (result->code_instructions == NULL) {
        puts("parse(): malloc failed (3)");
        free(result->data_instructions);
        free(result);
        return NULL;
    }
    result->labels = hashmap_create();
    if (result->labels == NULL) {
        puts("parse(): hashmap_create failed");
        free(result->code_instructions);
        free(result->data_instructions);
        free(result);
        return NULL;
    }
    result->memory_locations = hashmap_create();
    if (result->memory_locations == NULL) {
        puts("parse(): hashmap_create failed (2)");
        hashmap_destroy(result->labels);
        free(result->code_instructions);
        free(result->data_instructions);
        free(result);
        return NULL;
    }

    result->data_count = 0;
    result->code_count = 0;
    int data_capacity = DEFAULT_DATA_COUNT;
    int code_capacity = DEFAULT_CODE_COUNT;

    char line[MAX_LINE_LEN] = {0};
    int in_data_section = 0;
    int in_code_section = 0;

    while (fgets(line, sizeof(line), file))
    {
        // Supprimer le saut de ligne
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Vérifier si la ligne est vide
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
            if (result->data_count >= data_capacity) {
                data_capacity *= 2;
                result->data_instructions = realloc(result->data_instructions, sizeof(Instruction*) * data_capacity);
                if (result->data_instructions == NULL) {
                    puts("parse(): realloc failed");
                    hashmap_destroy(result->memory_locations);
                    hashmap_destroy(result->labels);
                    free(result->code_instructions);
                    free(result->data_instructions);
                    free(result);
                    fclose(file);
                    return NULL;
                }
            }
            Instruction* instruction = parse_data_instruction(line, result->memory_locations);
            if (instruction != NULL) {
                result->data_instructions[result->data_count++] = instruction;
            } else {
                printf("parse(): could not parse '%s'\n", line);
            }
        } else if (in_code_section) {
            if (result->code_count >= code_capacity) {
                code_capacity *= 2;
                result->code_instructions = realloc(result->code_instructions, sizeof(Instruction*) * code_capacity);
                if (result->code_instructions == NULL) {
                    puts("parse(): realloc failed");
                    hashmap_destroy(result->memory_locations);
                    hashmap_destroy(result->labels);
                    free(result->code_instructions);
                    free(result->data_instructions);
                    free(result);
                    fclose(file);
                    return NULL;
                }
            }
            Instruction* instruction = parse_code_instruction(line, result->labels, result->code_count);
            if (instruction != NULL) {
                result->code_instructions[result->code_count++] = instruction;
            } else {
                printf("parse(): could not parse '%s'\n", line);
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
    if ((str == NULL) || (*str == NULL) || (values == NULL)) return 0;
    int replaced = 0;
    char* input = *str;
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (values->table[i].key != NULL && values->table[i].key != TOMBSTONE)
        {
            char* key = values->table[i].key;
            int value = *(int*)values->table[i].value;

            char* substr = strstr(input, key);
            if (substr) {
                char replacement[64];
                snprintf(replacement, sizeof(replacement), "%d", value);

                int key_len = strlen(key);
                int repl_len = strlen(replacement);
                // int remain_len = strlen(substr + key_len);

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
    if (result == NULL) {
        return -1;
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

    return 0;
}