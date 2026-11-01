#include "file_explorer.h"
#include <ncurses.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_FILES 256
#define MAX_PATH 1024

typedef struct {
    char name[256];
    int is_dir;
} FileItem;

void run_file_explorer(void) {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);

    char cwd[MAX_PATH];
    int selected = 0;
    int top_idx = 0;

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) == NULL) break;

        DIR *dir = opendir(".");
        if (!dir) break;

        FileItem files[MAX_FILES];
        int file_count = 0;

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
            strncpy(files[file_count].name, entry->d_name, sizeof(files[file_count].name));

            DIR *test_dir = opendir(entry->d_name);
            if (test_dir) {
                files[file_count].is_dir = 1;
                closedir(test_dir);
            } else {
                files[file_count].is_dir = 0;
            }
            file_count++;
        }
        closedir(dir);

        if (selected >= file_count) selected = file_count > 0 ? file_count - 1 : 0;

        clear();

        attron(A_BOLD);
        mvprintw(0, 2, " explorer ");
        attroff(A_BOLD);
        mvprintw(1, 2, "Path: %s", cwd);
        mvprintw(2, 2, "===================================================");

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        int list_height = max_y - 6;

        for (int i = 0; i < file_count && i < list_height; i++) {
            int idx = top_idx + i;
            if (idx >= file_count) break;

            if (idx == selected) {
                attron(A_REVERSE);
            }

            if (files[idx].is_dir) {
                mvprintw(4 + i, 4, "==Dir== %s/", files[idx].name);
            } else {
                mvprintw(4 + i, 4, "==File== %s", files[idx].name);
            }

            if (idx == selected) {
                attroff(A_REVERSE);
            }
        }

        mvprintw(max_y - 2, 2, "[up_arrow/k, down_arrow/j] Move | [Return/Enter/l] Open | [h/backspace] Back | [q] Quit");
        refresh();

        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            break;
        } else if (ch == KEY_DOWN || ch == 'j') {
            if (selected < file_count - 1) {
                selected++;
                if (selected >= top_idx + list_height) top_idx++;
            }
        } else if (ch == KEY_UP || ch == 'k') {
            if (selected > 0) {
                selected--;
                if (selected < top_idx) top_idx--;
            }
        } else if (ch == '\n' || ch == KEY_ENTER || ch == 'l') {
            if (files[selected].is_dir) {
                chdir(files[selected].name);
                selected = 0;
                top_idx = 0;
            }
        } else if (ch == 'h' || ch == KEY_BACKSPACE) {
            chdir("..");
            selected = 0;
            top_idx = 0;
        }
    }

    endwin();
}

