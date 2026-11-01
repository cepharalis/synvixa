#pragma once

#include "term.h"

#define EXPLORER_NORMAL TERM_WHITE
#define EXPLORER_DIR (TERM_BLUE | TERM_BRIGHT)
#define EXPLORER_FILE TERM_WHITE

#define MAX_PATH_LEN 1024
#define MAX_ITEMS 256

typedef struct explorer_item {
    char name[256];
    bool is_dir;
} explorer_item;

typedef struct explorer {
    char cwd[MAX_PATH_LEN];
    explorer_item items[MAX_ITEMS];
    int item_count;
    int selected;
    int top_idx;
    bool dirty;
    bool active;
	int found;
} explorer;

explorer explorer_new();

bool explorer_update(explorer *e, int c);

void explorer_render(explorer *e, vec2 pos, vec2 size);