#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<ctype.h>

#define  vlavo 1
#define  vpravo 2
#define  hore 3
#define  dole 4

/* IZP Projekt 3 - Prace s datovymi strukturami
** autor: Patrik Demsky
** login: xdemsk00
** datum: 10.12.2019
*/

typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

int index_cells(Map *map, int r, int c) {
    //pocet riadkov pred tymto * pocet stlpcov v mape + stplec na ktorom sme - 1 pretoze pole od 0
    return ((r - 1) * map->cols + c - 1);
}
// kontrola hran
bool isborder(Map *map, int r, int c, int border) {
    int index = index_cells(map, r, c);
    switch (border) {
        case vlavo:
            if (map->cells[index] == 1 || map->cells[index] == 5 ||
                map->cells[index] == 3 || map->cells[index] == 7)
                return true;
            return false;
        case vpravo:
            if (map->cells[index] == 2 || map->cells[index] == 3 ||
                map->cells[index] == 6 || map->cells[index] == 7)
                return true;
            return false;
            //chceme cislo 2, 3, 6, 7
        case hore:
        case dole:
            if (map->cells[index] == 4 || map->cells[index] == 5 ||
                map->cells[index] == 6 || map->cells[index] == 7)
                return true;
            return false;
            // chceme cislo 4, 5, 6, 7
        default:
            return false;
    }
}


bool test_borders(Map *map) {
    for (int r = 1; r <= map->rows; r++) {
        for (int c = 1; c <= map->cols; c++) {
            // aky mam trojuholnik
            if ((c + r) % 2 == 0) {
                //trojuholnik \/
                //pre
                if (c > 1) {
                    if (isborder(map, r, c, 0) != isborder(map, r, c - 1, 1)) {
                        return false;
                    }
                }
                //pre /
                if (c < map->cols) {
                    if (isborder(map, r, c, 1) != isborder(map, r, c + 1, 0)) {
                        return false;
                    }
                }
                //pre _
                if (r > 1) {
                    if (isborder(map, r, c, 2) != isborder(map, r - 1, c, 2)) {
                        return false;
                    }
                }
            } else {
                //trojuholnik /
                //pre /
                if (c > 1) {
                    if (isborder(map, r, c, 0) != isborder(map, r, c - 1, 1)) {
                        return false;
                    }
                }
                //pre
                if (c < map->cols) {
                    if (isborder(map, r, c, 1) != isborder(map, r, c + 1, 0)) {
                        return false;
                    }
                }
                //pre _
                if (r < map->rows) {
                    if (isborder(map, r, c, 2) != isborder(map, r + 1, c, 2)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

// uvolnenie pamati
void destroy_map(Map *map) {
    free(map->cells);
    free(map);
}

Map *create_map(char filename[]) {
    //kontrola alokacie
    Map *my_map = malloc(sizeof(Map));
    if (my_map == NULL) {
        return NULL;
    }

    FILE *f = fopen(filename, "r");
    // overenie ci ide otvorit subor
    if (f == NULL) {
        free(my_map);
        fputs("failed to open file\n", stderr);
        return NULL;

    }

    char line[20];
    char *end;
    fgets(line, 20, f);
    if (line == NULL) {
        fputs("Error\n", stderr);
        return NULL;
    }

    my_map->rows = strtol(line, &end, 10);
    my_map->cols = strtol(end, &end, 10);
    for (int i = 0; end[i]; i++) {
        if (!isspace(end[i])) {
            fputs("Error: wrong first line\n", stderr);
            return NULL;
        }
    }
    // kontrola prveho riadku
    if (my_map->rows == 0 || my_map->cols == 0) {
        return NULL;
    }


    int num_cells = my_map->cols * my_map->rows;
    my_map->cells = malloc(num_cells * sizeof(unsigned char));
    //kontrola mallocu
    if (my_map->cells == NULL) {
        fclose(f);
        free(my_map);
        return NULL;
    }

    int znak;
    //prechadza kazdu bunku
    for (int r = 0; r < my_map->rows; r++) {
        for (int c = 0; c < my_map->cols; c++) {
            while (isspace(znak = fgetc(f)));
            if (znak == EOF) {
                // chyba riadok alebo stlpec
                return NULL;
            }
            // kontrola ci sa bludisko sklada zo znakov 0 - 7
            if (znak < '0' || znak > '7') {
                return NULL;
            }
            my_map->cells[my_map->cols * r + c] = znak - '0';
        }
        while ((znak = fgetc(f)) == '\t' || znak == ' ');
        if (znak == '\r') {
            fputs("Error\n", stderr);
            znak = fgetc(f);
        }
        // kontrola ci na konci riadku nie je nejaky nadbytocny znak
        if (znak != '\n' && znak != EOF) {
            return NULL;
        }
        ungetc(znak, f);
    }
    fclose(f);
    return my_map;
}

//hladanie cesty po bludisku
bool move(Map *map, int *r, int *c, int where) {
    printf("%d,%d\n", *r, *c);
    if (where == vlavo) {
        --(*c);
    } else if (where == vpravo) {
        ++(*c);
    } else if (((*c) + (*r)) % 2 == 0) {
        --(*r);
    } else {
        ++(*r);
    }
    if (*c == 0 || *r == 0 || *c > map->cols || *r > map->rows)
        return false;
    return true;
}


int wherenext(Map *map, int r, int c, int from, int leftright) {
    int where;
    while (1) {
        // vyhladavanie vsetkych moznosti kde mozem ist
        if (from == vlavo && leftright && (c + r) % 2 == 0)
            where = hore;
        else if (from == vpravo && leftright && (c + r) % 2 == 0)
            where = vlavo;
        else if (from == hore && leftright && (c + r) % 2 == 0)
            where = vpravo;
        else if (from == vlavo && !leftright && (c + r) % 2 == 0)
            where = vpravo;
        else if (from == vpravo && !leftright && (c + r) % 2 == 0)
            where = hore;
        else if (from == hore && !leftright && (c + r) % 2 == 0)
            where = vlavo;
        else if (from == vlavo && leftright && (c + r) % 2 != 0)
            where = vpravo;
        else if (from == vpravo && leftright && (c + r) % 2 != 0)
            where = dole;
        else if (from == dole && leftright && (c + r) % 2 != 0)
            where = vlavo;
        else if (from == vlavo && !leftright && (c + r) % 2 != 0)
            where = dole;
        else if (from == vpravo && !leftright && (c + r) % 2 != 0)
            where = vlavo;
        else if (from == dole && !leftright && (c + r) % 2 != 0)
            where = vpravo;
        else
            return 0;

        if (!isborder(map, r, c, where))
            return where;
        from = where;
    }
}

int start_border(Map *map, int r, int c, int leftright) {
    // hlada, ci ma bludisko vobec nejaky vstup
    if (r != 1 && c != 1 && map->rows != r && map->cols != c) {
        fprintf(stderr, "Error : Vstup do bludiska nenajdeny\n");
        return 1;
    }
    // skenovanie stran dokedy nenajde vstup
    if (c == 1 && !isborder(map, r, c, vlavo))
        return wherenext(map, r, c, vlavo, leftright);;
    if (c == map->cols && !isborder(map, r, c, vpravo))
        return wherenext(map, r, c, vpravo, leftright);
    if (r == 1 && !isborder(map, r, c, hore))
        if ((r + c) % 2 == 0)
            return wherenext(map, r, c, hore, leftright);
    if (r == map->rows && !isborder(map, r, c, hore))
        if ((r + c) % 2 == 1)
            return wherenext(map, r, c, dole, leftright);;

    return 1; //chyba

}

int main(int argc, char *argv[]) {
    //kontrola spravneho poctu argumentov
    if (argc < 2 || argc > 5) {
        printf("Nespravny pocet argumentov\n");
        return 1;
        // zadanie argumentu --help
    } else if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        printf("\n"
               "SYNTAX SPUSTENIA\n"
               "\n"
               "Program sa spusta v nasledujucej podobe :\n"
               "./proj3 --help\n"
               "./proj3 --test soubor.txt\n"
               "./proj3 --rpath R C soubor.txt\n"
               "./proj3 --lpath R C soubor.txt\n"
               "./proj3 --shortest R C soubor.txt\n"
               "--------------------------------------------------------------------------------------------------\n"
               "--help sposobi, ze program vytiskne napovedu pouzivania programu a skonci\n"
               "--test iba skontroluje, ze subor dany druhym argumentom programu obsahuje riadnu definiciu mapy bludiska.\n  "
               "V pripade, ze format obrazku odpoveda definicii, vytiskne Valid.\n  "
               "V opacnom pripade (napr. ilegalnych znakov, chybajucich udajov alebo zlych hodnot) program vytiskne Invalid.\n"
               "--rpath hlada priechod bludiskom na vstupe na riadku R a na stlpci C . Priechod hlada podla pravidla pravej ruky.\n"
               "--lpath hlada priechod bludiskom na vstupe na riadku R a na stlpci C . Priechod hlada podla pravidla lavej ruky.\n"
        return 0;
        //zadanie argumentu --test
    } else if (argc == 3 && strcmp(argv[1], "--test") == 0) {
        Map *map = create_map(argv[2]);

        if (map != NULL) {
            fputs("Valid\n", stderr);
            destroy_map(map);
        } else {
            fputs("Invalid\n", stderr);
        }
        //zadanie argumentu --rpath
    } else if (argc == 5 && strcmp(argv[1], "--rpath") == 0) {
        Map *map = create_map(argv[4]);
        char *endptr;
        int r = strtol(argv[2], &endptr, 10);
        if (*endptr) {
            fputs("Error", stderr);
        }
        int c = strtol(argv[3], &endptr, 10);
        if (*endptr) {
            fputs("Error", stderr);
        }
        int where = start_border(map, r, c, vpravo);
        while (move(map, &r, &c, where)) {
            int from = where;
            if (where == vlavo)
                from = vpravo;
            else if (where == vpravo)
                from = vlavo;
            else if (where == hore)
                from = dole;
            else if (where == dole)
                from = hore;
            where = wherenext(map, r, c, from, 0);
        }
        //zadanie argumentu --lpath
    } else if (argc == 5 && strcmp(argv[1], "--lpath") == 0) {
        Map *map = create_map(argv[4]);
        char *endptr;
        int r = strtol(argv[2], &endptr, 10);
        if (*endptr) {
            fputs("Error", stderr);
        }
        int c = strtol(argv[3], &endptr, 10);
        if (*endptr) {
            fputs("Error", stderr);
        }
        int where = start_border(map, r, c, vlavo);
        while (move(map, &r, &c, where)) {
            int from;
            if (where == vlavo)
                from = vpravo;
            else if (where == vpravo)
                from = vlavo;
            else if (where == hore)
                from = dole;
            else if (where == dole)
                from = hore;
            where = wherenext(map, r, c, from, 1);
        }
    }
    return 0;
}
