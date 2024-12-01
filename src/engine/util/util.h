#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "../types.h"

#define WHITE (vec4){1, 1, 1, 1}
#define DIM_WHITE (vec4){1, 1, 1, 0.1}
#define BLACK (vec4){0, 0, 0, 1}
#define RED (vec4){1, 0, 0, 1}
#define GREY (vec4){0.859, 0.859, 0.839, 1}
#define GREEN (vec4){0, 1, 0, 1}
#define BLUE (vec4){0, 0, 1, 1}
#define YELLOW (vec4){1, 1, 0, 1}
#define CYAN (vec4){0, 1, 1, 1}
#define MAGENTA (vec4){1, 0, 1, 1}
#define ORANGE (vec4){1, 0.5, 0, 1}
#define PURPLE (vec4){0.5, 0, 1, 1}
#define TURQUOISE (vec4){0, 0, 0.5, 1}

#define ERROR_EXIT(...) { fprintf(stderr, __VA_ARGS__); exit(1); }
#define ERROR_RETURN(R, ...) { fprintf(stderr, __VA_ARGS__); return R; }

typedef struct array_list {
    usize len;
    usize capacity;
    usize item_size;
    void *items;
} Array_List;

Array_List *array_list_create(usize item_size, usize initial_capacity);
usize array_list_append(Array_List *list, void *item);
void *array_list_get(Array_List *list, usize index);
u8 array_list_remove(Array_List *list, usize index);
char* concat(const char *s1, const char *s2);

