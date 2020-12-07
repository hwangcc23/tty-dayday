/*
 * vim:ts=4:sw=4:expandtab
 *
 * tty-dayday.c: Entry
 *
 * Arthur  Chih-Chyuan Hwang (hwangcc@csie.nctu.edu.tw)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include <getopt.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct dayday
{
    int bgcolor;
    int color;

    struct {
        char *str;
        /* struct tm *tm; */
    } event;

    WINDOW *msg_win;

    WINDOW *ymd_win;
    struct {
        int y, x;
        int w, h;
    } ymd_win_geo;

    bool running;
};

int ver_main = 0;
int ver_min = 1;
struct dayday dayday;

const struct option options[] =                                                                          
{
    { "help", 0, 0, 'h' },
    { "version", 0, 0, 'v' },
    { "event", 1, 0, 'e' },
    { "date", 1, 0, 'd' },
    { "since", 1, 0, 's' },
    { "until", 1, 0, 'u' },
    { NULL, 0, 0, 0 },
};

const char *optstring = "hve:d:su";

static int init(void)
{
    initscr();
    cbreak();
    noecho();
    start_color();
    curs_set(false);
    clear();

    init_pair(0, dayday.bgcolor, dayday.bgcolor);
    init_pair(1, dayday.bgcolor, dayday.color);
    init_pair(2, dayday.color, dayday.bgcolor);

    refresh();

    dayday.msg_win = newwin(1, strlen(dayday.event.str), 0, 0);
    if (!dayday.msg_win) {
        fprintf(stderr, "Failed to create the Window msg_win\n");
        return -1;
    }

    wrefresh(dayday.msg_win);

    dayday.running = true;

    return 0;
}

static void draw_windows(void)
{
    wbkgdset(dayday.msg_win, (COLOR_PAIR(2)));
    mvwaddstr(dayday.msg_win, 0, 0, dayday.event.str);
    wrefresh(dayday.msg_win);
}

static void get_keys(void)
{
    int key;

    switch (key = wgetch(stdscr)) {
    case 'q':
    case 'Q':
        dayday.running = false;
        break;

    default:
        break;
    }
}

int main(int argc, char ** argv)
{
    int longindex, c;

    memset(&dayday, 0, sizeof(struct dayday));
    dayday.color = COLOR_GREEN;
    dayday.bgcolor = COLOR_BLACK;

    for (;;) {
        c = getopt_long(argc, argv, optstring, options, &longindex);
        if (c == -1)
            break;

        switch (c) {
        case 'h':
            break;

        case 'v':
            fprintf(stdout, "Version %d.%d\n", ver_main, ver_min);
            break;

        case 'e':
            dayday.event.str = strdup(optarg);
            break;

        case 'd':
            break;

        case 's':
            break;

        case 'u':
            break;

        default:
            fprintf(stderr, "Unknown argument: %c\n", c);
            break;
        }
    }

    if (argc != optind || !dayday.event.str) {
        fprintf(stderr, "Invalid arguments or no input event\n");
        return EXIT_FAILURE;
    }

    init();

    while (dayday.running) {
        draw_windows();
        get_keys();
    }

    if (dayday.msg_win)
        delwin(dayday.msg_win);
    if (dayday.ymd_win)
        delwin(dayday.ymd_win);

    endwin();

    return EXIT_SUCCESS;
}
