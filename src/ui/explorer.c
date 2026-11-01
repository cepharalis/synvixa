#include "explorer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

static int explorer_item_cmp(const void* a, const void* b) {
	return strcmp(((const explorer_item*) a)->name, ((const explorer_item*) b)->name);
}

explorer explorer_new() {
    explorer e;
    e.selected = 0;
    e.top_idx = 0;
    e.item_count = 0;
    e.dirty = true;
    e.active = true;
	e.found = -1;

    if (getcwd(e.cwd, sizeof(e.cwd)) == NULL) {
        strcpy(e.cwd, ".");
    }

    DIR* dir = opendir(".");
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL && e.item_count < MAX_ITEMS) {
            strncpy(e.items[e.item_count].name, entry->d_name, sizeof(e.items[e.item_count].name));

            DIR *test_dir = opendir(entry->d_name);
            if (test_dir) {
                e.items[e.item_count].is_dir = true;
                closedir(test_dir);
            } else {
                e.items[e.item_count].is_dir = false;
            }
            e.item_count++;
        }
        closedir(dir);

    	qsort(e.items, e.item_count, sizeof(explorer_item), explorer_item_cmp);
    }

    return e;
}

static void explorer_refresh_dir(explorer *e) {
    if (getcwd(e->cwd, sizeof(e->cwd)) == NULL) return;

    DIR *dir = opendir(".");
    if (!dir) return;

    e->item_count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && e->item_count < MAX_ITEMS) {
        strncpy(e->items[e->item_count].name, entry->d_name, sizeof(e->items[e->item_count].name));

        DIR *test_dir = opendir(entry->d_name);
        if (test_dir) {
            e->items[e->item_count].is_dir = true;
            closedir(test_dir);
        } else {
            e->items[e->item_count].is_dir = false;
        }
        e->item_count++;
    }
    closedir(dir);

	qsort(e->items, e->item_count, sizeof(explorer_item), explorer_item_cmp);

    if (e->selected >= e->item_count) {
        e->selected = e->item_count > 0 ? e->item_count - 1 : 0;
    }
    e->top_idx = 0;
    e->dirty = true;
}

bool explorer_update(explorer *e, const int c) {
    bool dirty = false;

    if (c == TERM_DOWN_ARROW || c == 'j') {
        if (e->selected < e->item_count - 1) {
            e->selected++;
            e->dirty = true;
            dirty = true;
        }
    }
    else if (c == TERM_UP_ARROW || c == 'k') {
        if (e->selected > 0) {
            e->selected--;
            e->dirty = true;
            dirty = true;
        }
    }
    else if (c == '\n' || c == TERM_RIGHT_ARROW || c == 'l') {
        if (e->item_count > 0) {
        	if (e->items[e->selected].is_dir) {
        		chdir(e->items[e->selected].name);
        		explorer_refresh_dir(e);
        	}
        	else {
        		e->found = e->selected;
        	}
            dirty = true;
        }
    }
    else if (c == TERM_LEFT_ARROW || c == 'h') {
        chdir("..");
        explorer_refresh_dir(e);
        dirty = true;
    }
    else if (c == 'q' || c == 'Q') {
        e->active = false;
        dirty = true;
    }

    return dirty;
}

void explorer_render(explorer *e, const vec2 pos, const vec2 size) {
    term_set_cursor((vec2){.x = pos.x, .y = pos.y});
    term_write_c(" explorer ", TERM_WHITE | TERM_BRIGHT);

    term_set_cursor((vec2){.x = pos.x, .y = pos.y + 1});
    term_write_lc(e->cwd, strlen(e->cwd), TERM_WHITE);

    const int display_limit = (int) size.y - 3;
    if (e->selected < e->top_idx) e->top_idx = e->selected;
    if (e->selected >= e->top_idx + display_limit) e->top_idx = e->selected - display_limit + 1;

    for (unsigned int i = 0; i < (unsigned int)display_limit && i < (unsigned int)e->item_count; i++) {
        const unsigned int idx = e->top_idx + i;
        term_set_cursor((vec2){.x = pos.x, .y = pos.y + 2 + i});

        const unsigned int colour = e->items[idx].is_dir ? EXPLORER_DIR : EXPLORER_FILE;

        if (idx == e->selected) {
            term_write_cs(">", colour, TERM_UNDERLINE);
        } else {
            term_write_c(" ", EXPLORER_NORMAL);
        }

        char display_buf[300];
        snprintf(display_buf, sizeof(display_buf), " %s%s", e->items[idx].name, e->items[idx].is_dir ? "/" : "");
        term_write_lc(display_buf, strlen(display_buf), colour);
    }
    e->dirty = false;
}