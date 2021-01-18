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
#include <limits.h>
#include <math.h>
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
        char *name;              /* Name of the event */
        struct tm date;         /* Date of the event */
        unsigned int dy, dm, dd, days;   /* delta year/mon/day and delta days */
    } event;

    WINDOW *msg_win;

    WINDOW *ymd_win;
    struct {
        int y, x;
        int w, h;
    } ymd_win_geo;

    WINDOW *days_win;
    struct {
        int y, x;
        int w, h;
    } days_win_geo;

    WINDOW *help_win;
    struct {
        int y, x;
        int w, h;
    } help_win_geo;

    bool running;
    bool count_since;
    bool hide_help;
    bool ymd_format;
};

#define NR_COLORS 8
#define DAYDAY_DIGIT_COLOR_0 0
#define DAYDAY_DIGIT_COLOR_1 1
#define DAYDAY_NAME_COLOR 2
#define DAYDAY_HELP_COLOR 3
#define DAYDAY_BG_COLOR 4
#define DIGIT_WIDTH 6
#define DIGIT_HEIGHT 5
#define YMD_WIN_HEIGHT 7
#define YMD_WIN_WIDTH 60
#define DAYDAY_GET_KEY_DELAY_SEC 60

int ver_main = 0;
int ver_min = 4;
struct dayday dayday;
const char *msg_in_help_win = "Press q/Q key to quit, t/T key to Tint, f/F key to change the format, h/H key to hide these help messages";

const struct option options[] =
{
    { "help", 0, 0, 'h' },
    { "version", 0, 0, 'v' },
    { "event", 1, 0, 'e' },
    { "date", 1, 0, 'd' },
    { "since", 1, 0, 's' },
    { "until", 1, 0, 'u' },
    { "tint", 1, 0, 't' },
    { NULL, 0, 0, 0 },
};

const char *optstring = "hve:d:sut:";

static const int mon_days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

const int digit_pixels[][15] =
{
     {1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1}, /* 0 */
     {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1}, /* 1 */
     {1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1}, /* 2 */
     {1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1}, /* 3 */
     {1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1}, /* 4 */
     {1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1}, /* 5 */
     {1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1}, /* 6 */
     {1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1}, /* 7 */
     {1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1}, /* 8 */
     {1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1}, /* 9 */
};

static void usage(void)
{
    fprintf(stdout, "Usage: tty-dayday -e NAME_OF_EVENT - d DATE_IN_MM/DD/YYYY [options]\n");
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "  -s|--since            Count days SINCE the given date (default option)\n");
    fprintf(stdout, "  -u|--until            Count days until the given date\n");
    fprintf(stdout, "  -t|--tint [COLOR]     Color the font and numbers\n");
    fprintf(stdout, "                        COLOR = 1 -> Red\n");
    fprintf(stdout, "                        COLOR = 2 -> Green\n");
    fprintf(stdout, "                        COLOR = 3 -> Yellow\n");
    fprintf(stdout, "                        COLOR = 4 -> Blue\n");
    fprintf(stdout, "                        COLOR = 5 -> Magenta\n");
    fprintf(stdout, "                        COLOR = 6 -> Cyan\n");
    fprintf(stdout, "                        COLOR = 7 -> White\n");
    fprintf(stdout, "  -v|--version          Print the version number and exit\n");
    fprintf(stdout, "  -h|--help             Print the help messages and exit\n");
}

static void tint(int color)
{
    color %= NR_COLORS;

    switch (color) {
    case 1:
        dayday.color = COLOR_RED;
        break;

    case 2:
        dayday.color = COLOR_GREEN;
        break;

    case 3:
        dayday.color = COLOR_YELLOW;
        break;

    case 4:
        dayday.color = COLOR_BLUE;
        break;

    case 5:
        dayday.color = COLOR_MAGENTA;
        break;

    case 6:
        dayday.color = COLOR_CYAN;
        break;

    case 7:
        dayday.color = COLOR_WHITE;
        break;

    default:
        break;
    }

    if (dayday.running) {
        init_pair(DAYDAY_NAME_COLOR, dayday.color, dayday.bgcolor);
        init_pair(DAYDAY_DIGIT_COLOR_1, dayday.bgcolor, dayday.color);
    }
}

static int count_num_digit(unsigned int number)
{
    int cnt = 0;

    if (!number)
        return 1;

    while (number) {
        cnt++;
        number /= 10;
    }

    return cnt;
}

static int init_windows(void)
{
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, true);
    start_color();
    curs_set(false);
    clear();

    init_pair(DAYDAY_NAME_COLOR, dayday.color, dayday.bgcolor);
    init_pair(DAYDAY_DIGIT_COLOR_0, dayday.bgcolor, dayday.bgcolor);
    init_pair(DAYDAY_DIGIT_COLOR_1, dayday.bgcolor, dayday.color);
    init_pair(DAYDAY_HELP_COLOR, COLOR_WHITE, COLOR_RED);
    init_pair(DAYDAY_BG_COLOR, dayday.bgcolor, dayday.bgcolor);

    refresh();

    dayday.ymd_win_geo.y = 1;
    dayday.ymd_win_geo.x = 0;
    dayday.ymd_win_geo.w = YMD_WIN_WIDTH;
    dayday.ymd_win_geo.h = YMD_WIN_HEIGHT;

    dayday.days_win_geo.y = 1;
    dayday.days_win_geo.x = 0;
    dayday.days_win_geo.w = YMD_WIN_WIDTH;
    dayday.days_win_geo.h = YMD_WIN_HEIGHT;

    dayday.help_win_geo.y = LINES - 1;
    dayday.help_win_geo.x = 0;
    dayday.help_win_geo.w = COLS;
    dayday.help_win_geo.h = 1;

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

    dayday.days_win = newwin(dayday.days_win_geo.h, dayday.days_win_geo.w, dayday.days_win_geo.y, dayday.days_win_geo.x);
    if (!dayday.days_win) {
        fprintf(stderr, "Failed to create the Window days_win\n");
        return -1;
    }

    dayday.help_win = newwin(dayday.help_win_geo.h, dayday.help_win_geo.w, dayday.help_win_geo.y, dayday.help_win_geo.x);
    if (!dayday.help_win) {
        fprintf(stderr, "Failed to create the Window help_win\n");
        return -1;
    }

    wrefresh(dayday.msg_win);
    wrefresh(dayday.ymd_win);
    wrefresh(dayday.days_win);
    wrefresh(dayday.help_win);

    return 0;
}

static void draw_digit_in_window(WINDOW *win, int y, int x, int digit)
{
    int i, sx;

    for (i = 0, sx = x; i < (DIGIT_WIDTH * DIGIT_HEIGHT); i++, sx++) {
        if (sx == x + DIGIT_WIDTH) {
            sx = x;
            y++;
        }

        wbkgdset(win, COLOR_PAIR(digit_pixels[digit][i / 2] ? DAYDAY_DIGIT_COLOR_1 : DAYDAY_DIGIT_COLOR_0));
        mvwaddch(win, y, sx, ' ');
    }
}

static void draw_windows(void)
{
    int digit, pre_digit;
    int i;
    int nr_digit, x;

    /*
     * Draw the message window.
     */

    wbkgdset(dayday.msg_win, (COLOR_PAIR(DAYDAY_NAME_COLOR)));
    mvwaddstr(dayday.msg_win, 0, 0, dayday.event.name);

    wrefresh(dayday.msg_win);

    /*
     * Clear YYYY/MM/DD and Days window.
     */

    wbkgdset(dayday.ymd_win, (COLOR_PAIR(DAYDAY_BG_COLOR)));
    wclear(dayday.ymd_win);
    wrefresh(dayday.ymd_win);
    wbkgdset(dayday.days_win, (COLOR_PAIR(DAYDAY_BG_COLOR)));
    wclear(dayday.days_win);
    wrefresh(dayday.days_win);

    /*
     * Draw the YYYY/MM/DD window.
     */

    if (dayday.ymd_format) {
        wbkgdset(dayday.ymd_win, (COLOR_PAIR(DAYDAY_NAME_COLOR)));

        mvwaddstr(dayday.ymd_win, 0, 8, "YEAR");

        /*
         * Draw numbers to show the counting days.
         * On drawing digits of the number, the zero in the number should be ignore
         * to avoid users' confusion. For example, draw "  17" rather than "0017".
         * In the implementation below, use digit and pre_digit to check whether
         * the zero digit should be ignored or not.
         */

        digit = dayday.event.dy / 1000;
        if (digit)
            draw_digit_in_window(dayday.ymd_win, 1, 1, digit);
        pre_digit = digit;
        digit = (dayday.event.dy % 1000 ) / 100;
        if (digit || (pre_digit && !digit))
            draw_digit_in_window(dayday.ymd_win, 1, 1 + 1 + DIGIT_WIDTH, digit);
        pre_digit = pre_digit * 10 + digit;
        digit = (dayday.event.dy % 100) / 10;
        if (digit || (pre_digit && !digit))
            draw_digit_in_window(dayday.ymd_win, 1, 1 + 1 * 2 + DIGIT_WIDTH * 2, digit);
        digit = dayday.event.dy % 10;
        draw_digit_in_window(dayday.ymd_win, 1, 1 + 1 * 3 + DIGIT_WIDTH * 3, digit);

        wbkgdset(dayday.ymd_win, (COLOR_PAIR(DAYDAY_NAME_COLOR)));
        mvwaddstr(dayday.ymd_win, DIGIT_HEIGHT, 1 + 4 * 1 + 4 * DIGIT_WIDTH, "/");

        wbkgdset(dayday.ymd_win, (COLOR_PAIR(DAYDAY_NAME_COLOR)));
        mvwaddstr(dayday.ymd_win, 0, 31, "MONTH");

        digit = dayday.event.dm / 10;
        if (digit)
            draw_digit_in_window(dayday.ymd_win, 1, 31, digit);
        digit = dayday.event.dm % 10;
        draw_digit_in_window(dayday.ymd_win, 1, 31 + 1 + DIGIT_WIDTH, digit);

        wbkgdset(dayday.ymd_win, (COLOR_PAIR(DAYDAY_NAME_COLOR)));
        mvwaddstr(dayday.ymd_win, DIGIT_HEIGHT, 31 + 2 * 1 + 2 * DIGIT_WIDTH, "/");

        wbkgdset(dayday.ymd_win, (COLOR_PAIR(DAYDAY_NAME_COLOR)));
        mvwaddstr(dayday.ymd_win, 0, 47, "DAY");

        digit = dayday.event.dd / 10;
        if (digit)
            draw_digit_in_window(dayday.ymd_win, 1, 47, digit);
        digit = dayday.event.dd % 10;
        draw_digit_in_window(dayday.ymd_win, 1, 47 + 1 + DIGIT_WIDTH, digit);

        wrefresh(dayday.ymd_win);
    }

    /*
     * Draw the Days window.
     */

    if (!dayday.ymd_format) {
        nr_digit = count_num_digit(dayday.event.days);
        x = 1;

        digit = dayday.event.days / (unsigned int)pow(10, nr_digit - 1);
        if (digit)
            draw_digit_in_window(dayday.days_win, 1, x, digit);
        pre_digit = digit;

        for (i = nr_digit - 1; i > 1; i--) {
            digit = (dayday.event.days % (unsigned int)pow(10, i) ) / (unsigned int)pow(10, i - 1);
            if (digit || (pre_digit && !digit)) {
                x += 1 + DIGIT_WIDTH;
                draw_digit_in_window(dayday.days_win, 1, x, digit);
            }
            pre_digit = pre_digit * 10 + digit;
        }

        digit = dayday.event.days % 10;
        x += 1 + DIGIT_WIDTH;
        draw_digit_in_window(dayday.days_win, 1, x, digit);

        wbkgdset(dayday.days_win, (COLOR_PAIR(DAYDAY_NAME_COLOR)));
        mvwaddstr(dayday.days_win, 0, x + DIGIT_WIDTH - 4, "DAYS");

        wrefresh(dayday.days_win);
    }

    /*
     * Draw the Help window.
     */

    if (dayday.hide_help) {
        wbkgdset(dayday.help_win, (COLOR_PAIR(DAYDAY_BG_COLOR)));

        wclear(dayday.help_win);
    } else {
        wbkgdset(dayday.help_win, (COLOR_PAIR(DAYDAY_HELP_COLOR)));

        mvwaddstr(dayday.help_win, 0, 0, msg_in_help_win);
        for (i = strlen(msg_in_help_win); i < COLS; i++)
            mvwaddch(dayday.help_win, 0, i, ' ');
    }

    wrefresh(dayday.help_win);
}

static void get_keys(void)
{
    fd_set fds;
    struct timeval timeout;
    int ret, key, color;

    /*
     * In no-delay mode, if no input is waiting, wgetch() returns ERR directly.
     * To avoid occupying the CPU resource, use select() to monitor an input
     * from STDIN.
     */

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    timeout.tv_sec = DAYDAY_GET_KEY_DELAY_SEC;
    timeout.tv_usec = 0;

    ret = select(1, &fds, NULL, NULL, &timeout);
    if (ret <= 0)
        return ;

    switch (key = wgetch(stdscr)) {
    case 'q':
    case 'Q':
        dayday.running = false;
        break;

    case 't':
    case 'T':
        /*
         * Use the next colors (in the order of COLOR numbers) to tint fonts.
         * Since only numbers 1 ~ (NR_COLORS - 1) are valid,
         * add a check for bypassing the number 0.
         */
        color = (dayday.color + 1) % NR_COLORS;
        if (!color)
            color = 1;
        tint(color);
        break;

    case 'h':
    case 'H':
        dayday.hide_help = dayday.hide_help ? false : true;
        break;

    case 'f':
    case 'F':
        dayday.ymd_format = dayday.ymd_format ? false : true;
        break;

    default:
        break;
    }
}

static int count_leap_years(struct tm *date)
{
    int years = date->tm_year + 1900;

    if ((date->tm_mon + 1) <= 2)
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

    d1 = (date->tm_year + 1900) * 365 + date->tm_mday;
    for (i = 0; i < date->tm_mon; i++)
        d1 += mon_days[i];
    d1 += count_leap_years(date);

    d2 = (local->tm_year + 1900) * 365 + local->tm_mday;
    for (i = 0; i < local->tm_mon; i++)
        d2 += mon_days[i];
    d2 += count_leap_years(local);

    if (dayday.count_since && (d2 >= d1))
        dayday.event.days = d2 - d1;
    else if (!dayday.count_since && (d1 >= d2))
        dayday.event.days = d1 - d2;
    else
        dayday.event.days = 0;
    dayday.event.dy = dayday.event.days / 365;
    dayday.event.dm = (dayday.event.days % 365) / 30;
    dayday.event.dd = (dayday.event.days % 365) % 30;
}

int main(int argc, char ** argv)
{
    int longindex, c;
    int ret, mm, dd, yy;
    char *endptr;
    long val;

    memset(&dayday, 0, sizeof(struct dayday));
    dayday.color = COLOR_GREEN;
    dayday.bgcolor = COLOR_BLACK;
    dayday.count_since = true;

    for (;;) {
        c = getopt_long(argc, argv, optstring, options, &longindex);
        if (c == -1)
            break;

        switch (c) {
        case 'h':
            usage();
            return EXIT_SUCCESS;
            break;

        case 'v':
            fprintf(stdout, "Version %d.%d\n", ver_main, ver_min);
            return EXIT_SUCCESS;
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
            dayday.count_since = true;
            break;

        case 'u':
            dayday.count_since = false;
            break;

        case 't':
            val = strtol(optarg, &endptr, 0);
            if (val == LONG_MAX || val == LONG_MIN || val < 0 || *endptr != '\0' || endptr == optarg)
                fprintf(stderr, "Invalid argument %s\n", optarg);
            else {
                tint(val);
            }
            break;

        default:
            fprintf(stderr, "Unknown argument: %c\n", c);
            break;
        }
    }

    if (!dayday.event.date.tm_year || !dayday.event.date.tm_mday) {
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

    dayday.hide_help = false;
    dayday.ymd_format = true;
    dayday.running = true;

    while (dayday.running) {
        count_days();
        draw_windows();
        get_keys();
    }

    if (dayday.msg_win)
        delwin(dayday.msg_win);
    if (dayday.ymd_win)
        delwin(dayday.ymd_win);
    if (dayday.days_win)
        delwin(dayday.days_win);
    if (dayday.help_win)
        delwin(dayday.help_win);

    endwin();

    return EXIT_SUCCESS;
}
