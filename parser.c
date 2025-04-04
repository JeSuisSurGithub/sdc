#include "parser.h"
#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

Instruction *parse_data_instruction(const char *line, HashMap *memory_locations) {
    Instruction *inst = (Instruction*)malloc(sizeof(Instruction));
    if (!inst) return NULL;

    char nom[100], type[10], valeur[256];

    // Format : nom type valeurs
    int n = sscanf(line, "%s %s %[^\n]", nom, type, valeur); // %[^\n]: Lis tous les caractères jusqu’à rencontrer un saut de ligne (\n)
    if (n != 3) {
        free(inst);
        return NULL;
    }

    // Stocker dans l'instruction FAIRE FREE
    inst->mnemonic = strdup(nom);
    inst->operand1 = strdup(type);
    inst->operand2 = strdup(valeur);

    // Calculer combien de valeurs il y a dans operand2 (compter les virgules)
    static int adresse_suiv = 0;
    int s = 1;
    for (int i = 0; valeur[i]; i++) {
        if (valeur[i] == ',') s++;
    }

    // Enregistrer l’adresse dans la table de hachage
    int* adresse_suiv_mem = malloc(sizeof(int));
    (*adresse_suiv_mem) = adresse_suiv;
    hashmap_insert(memory_locations, inst->mnemonic, adresse_suiv_mem);
    adresse_suiv += s;

    return inst;
}

Instruction* parse_code_instruction(const char* line, HashMap* labels, int code_count) {
    Instruction* inst = (Instruction*)malloc(sizeof(Instruction));
    if (!inst) return NULL;

    char label[100] = "", mnemonic[20] = "", op1[100] = "", op2[100] = "";
    int i = 0;

    // Étape 1 : détecter s'il y a un label
    while (line[i] != '\0' && line[i] != ':' && i < 99) {
        label[i] = line[i];
        i++;
    }

    if (line[i] == ':') {
        label[i] = '\0'; // Fin de chaîne
        i++; // Sauter le ":"

        int* code_count_mem = malloc(sizeof(int));
        (*code_count_mem) = code_count;
        hashmap_insert(labels, label, code_count_mem);
    } else {
        // Pas de label, réinitialisez l'index à 0
        i = 0;
    }

    // Ignorer les espaces après le label
    while (line[i] == ' ') i++;

    // Étape 2 : lire le reste de la ligne (instruction)
    int n = sscanf(line + i, "%s %[^,],%[^\n]", mnemonic, op1, op2);

    // Vérifiez si on a trouvé une instruction valide
    if (n < 1) { // Si aucune instruction n'a été trouvée
        free(inst);
        return NULL; // Ligne ignorée
    }

    // Vérifier si l'instruction a un format valide
    inst->mnemonic = strdup(mnemonic);

    // Si l'instruction n'a pas d'opérandes, on remplit les opérandes avec des chaînes vides
    inst->operand1 = (n >= 2 && op1[0] != '\0') ? strdup(op1) : strdup("");
    inst->operand2 = (n == 3 && op2[0] != '\0') ? strdup(op2) : strdup("");

    return inst;
}



ParserResult* parse(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        return NULL;
    }
    ParserResult* result = malloc(sizeof(ParserResult));
    result->data_instructions = malloc(sizeof(Instruction*) * 10);
    result->code_instructions = malloc(sizeof(Instruction*) * 10);
    result->labels = hashmap_create();
    result->memory_locations = hashmap_create();
    result->data_count = 0;
    result->code_count = 0;
    int data_capacite = 10, code_capacite = 10;

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
                    perror("Erreur de réallocation de mémoire");
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
                    perror("Erreur de réallocation de mémoire");
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

