# SDC

## Compilation
1. Installer gcc et make
2. `make all`

## Utilisation
* `./sdc_main [FICHIER_ASM] [TAILLE_MEMOIRE]`
+ La taille mémoire doit-etre suffisante pour accomoder les variables et instructions du programme.
* Une instruction vaut une case, une variable simple vaut une aussi et pour tableau il s'agit de la taille du tableau

## Description
+ Makefile: Comporte les différentes cibles pour les tests et le programme final
* cpu.c/cpu.h: Contient les fonctions relatives au CPU
+ exemple.asm: Le fichier ASM pour tester toute les instructions
* hash.c/hash.h: Implémentation de table de hachage générique
+ main.c: Interface utilisateur (voir guide ci-dessous)
* memory.c/memory.h: Fonctions relatives à la gestion des segments de mémoire
+ parser.cparser.h: Analyse et traitement des fichiers ASM
* perf_boucle.asm: Fichier test ASM pour la vitesse du CPU
+ perf_compile.asm: Fichier test ASM pour la vitesse du parser
* test_cpu.c/test_cpu2.c/test_cpu3.c: Test progressif des différentes fonctionalités du CPU
+ test_es_segment.c: Test de la fonctionalité du extra segment
* test_hash.c: Test de la table de hachage
+ test_mem.c: Test du gestionnaire de segments
* test_parser.c: Test du parser
+ test_perf_cpu.c: Test de la performance du CPU
* test_perf_parser.c: Test de la performance du parser

## Guide d'usage

### Architecture

| Registre | Description |
|------------|------------|
| AX | Registre général A |
| BX | Registre général B |
| CX | Registre général C |
| DX | Registre général D |
| IP | Pointeur d'instruction (index du code) |
| ZF | Drapeau Zéro (vaut 1 si le résultat est zéro) |
| SF | Drapeau Signe (vaut 1 si le résultat est négatif) |
| SP | Pointeur de pile |
| BP | Pointeur de base (base de la pile) |
| ES | Segment supplémentaire (allocation dynamique) |

| Segment | Usage |
|------------|------------|
| CS | Segment de code (instructions) |
| DS | Segment de données (variables globales) |
| SS | Segment de pile (pile locale) |
| ES | Segment supplémentaire (tas / dynamique) |

| Mnémonique | Description |
|------------|------------|
| MOV dst, src | Copie la valeur de src vers dst |
| ADD dst, src | Ajoute src à dst |
| CMP dst, src | Compare dst et src, met à jour ZF/SF |
| JMP addr | Saut inconditionnel vers l’adresse addr |
| JZ addr | Saut vers addr si ZF == 1 |
| JNZ addr | Saut vers addr si ZF == 0 |
| HALT | Arrête l’exécution |
| PUSH val | Empile une valeur sur la pile |
| POP reg | Dépile la pile dans un registre |
| ALLOC | Alloue un segment mémoire dans ES (taille = AX, stratégie* = BX) |
| FREE | Libère le segment ES |

+ *Stratégie 0 opte pour le premier segment convenable, 1 pour celui avec la taille la plus proche et 2 la taille la plus grande

### Modes d'adressage

42 – Valeur immédiate

AX, BX, etc. – Accès direct à un registre

[5] – Accès direct à une adresse mémoire dans DS

[AX] – Accès indirect via registre (index dans DS)

[DS:AX], [SS:BX] – Préfixe de segment (accès mémoire via un segment spécifique)