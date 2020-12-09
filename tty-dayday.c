/*
 * vim:ts=4:sw=4:expandtab
 *
 * tty-dayday.c: Countdown days Since and Until. Draw days using ncurses.
 *
 * Arthur: Chih-Chyuan Hwang (hwangcc@csie.nctu.edu.tw)
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
#include <time.h>
#include <unistd.h>

struct dayday
{
    int bgcolor;
    int color;

    struct {
        char *name;              /* Name of the event to countdown */
        struct tm date;         /* Date of the event */
        int dy, dm, dd, days;   /* delta year/mon/day and delta days */
    } event;

    WINDOW *msg_win;

    WINDOW *ymd_win;
    struct {
        int y, x;
        int w, h;
    } ymd_win_geo;

    bool running;
};

#define DAYDAY_DIGIT_COLOR_0 0
#define DAYDAY_DIGIT_COLOR_1 1
#define DAYDAY_NAME_COLOR 2
#define YMD_WIN_HEIGHT 22
#define YMD_WIN_WIDTH 60

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

static const int mon_days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static int init_windows(void)
{
    initscr();
    cbreak();
    noecho();
    start_color();
    curs_set(false);
    clear();

    init_pair(DAYDAY_NAME_COLOR, dayday.color, dayday.bgcolor);
    init_pair(DAYDAY_DIGIT_COLOR_0, dayday.bgcolor, dayday.bgcolor);
    init_pair(DAYDAY_DIGIT_COLOR_1, dayday.bgcolor, dayday.color);

    refresh();

    dayday.ymd_win_geo.y = 1;
    dayday.ymd_win_geo.x = 0;
    dayday.ymd_win_geo.w = YMD_WIN_WIDTH;
    dayday.ymd_win_geo.h = YMD_WIN_HEIGHT;

    dayday.msg_win = newwin(1, strlen(dayday.event.name), dayday.ymd_win_geo.y - 1, 0);
    if (!dayday.msg_win) {
        fprintf(stderr, "Failed to create the Window msg_win\n");
        return -1;
    }

    dayday.ymd_win = newwin(dayday.ymd_win_geo.h, dayday.ymd_win_geo.w, dayday.ymd_win_geo.y, dayday.ymd_win_geo.x);
    if (!dayday.ymd_win) {
        fprintf(stderr, "Failed to create the Window ymd_win\n");
        return -1;
    }

    wrefresh(dayday.msg_win);
    wrefresh(dayday.ymd_win);

    dayday.running = true;

    return 0;
}

static void draw_windows(void)
{
    wbkgdset(dayday.msg_win, (COLOR_PAIR(DAYDAY_NAME_COLOR)));
    mvwaddstr(dayday.msg_win, dayday.ymd_win_geo.y - 1, 0, dayday.event.name);
    wrefresh(dayday.msg_win);

    wbkgdset(dayday.ymd_win, (COLOR_PAIR(DAYDAY_NAME_COLOR)));
    mvwaddstr(dayday.ymd_win, 0, 8, "YEAR");
    mvwaddstr(dayday.ymd_win, 0, 31, "MONTH");
    mvwaddstr(dayday.ymd_win, 0, 47, "DAY");
    wrefresh(dayday.ymd_win);
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

static int count_leap_years(struct tm *date)
{
    int years = date->tm_year;

    if (date->tm_mon <= 2)
        years--;

    return years / 4 - years / 100 + years / 400;
}

static void count_days(void)
{
    time_t raw;
    struct tm *date;
    struct tm *local;
    long d1, d2;
    int i;

    date = &dayday.event.date;

    time(&raw);
    local = localtime(&raw);

    d1 = date->tm_year * 365 + date->tm_mday;
    for (i = 0; i < date->tm_mon; i++)
        d1 += mon_days[date->tm_mon];
    d1 += count_leap_years(date);

    d2 = local->tm_year * 365 + local->tm_mday;
    for (i = 0; i < local->tm_mon; i++)
        d2 += mon_days[local->tm_mon];
    d2 += count_leap_years(local);

    dayday.event.days = d2 - d1;
    dayday.event.dy = dayday.event.days / 365;
    dayday.event.dm = (dayday.event.days % 365) / 30;
    dayday.event.dd = (dayday.event.days % 365) % 30;
}

int main(int argc, char ** argv)
{
    int longindex, c;
    int ret, mm, dd, yy;

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
            dayday.event.name = strdup(optarg);
            break;

        case 'd':
            ret = sscanf(optarg, "%d/%d/%d", &mm, &dd, &yy);
            if (ret != 3) {
                fprintf(stderr, "Invalid format of the date argument: %s\n", optarg);
                return EXIT_FAILURE;
            } else {
                dayday.event.date.tm_year = yy - 1900;
                dayday.event.date.tm_mon = mm - 1;
                dayday.event.date.tm_mday = dd;
            }
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

    if (!dayday.event.date.tm_year || !dayday.event.date.tm_mon || !dayday.event.date.tm_mday) {
        fprintf(stderr, "Date is not given\n");
        return EXIT_FAILURE;
    }
    if (!dayday.event.name) {
        fprintf(stderr, "Event is not given\n");
        return EXIT_FAILURE;
    }
    if (argc != optind) {
        fprintf(stderr, "Invalid arguments\n");
        return EXIT_FAILURE;
    }

    init_windows();

    while (dayday.running) {
        count_days();
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
