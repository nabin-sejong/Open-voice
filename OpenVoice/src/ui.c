/*
 * OpenVoice AAC Communicator
 * src/ui.c - ncurses User Interface Implementation
 *
 * Implements the terminal UI: main menu, phrase board, voice-settings
 * editor, text-input box, modal dialogues, and info screens.
 *
 * Standard: C99
 */

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/ui.h"
#include "../include/phrase.h"
#include "../include/profile.h"

/* -------------------------------------------------------------------------
 * Internal colour-pair IDs
 * ---------------------------------------------------------------------- */
#define CP_NORMAL       1   /* white on dark blue  - default text          */
#define CP_HIGHLIGHT    2   /* black on bright cyan - selected item        */
#define CP_TITLE        3   /* bright yellow on dark blue - headers        */
#define CP_BORDER       4   /* bright white on dark blue - box borders     */
#define CP_STATUS       5   /* black on bright green - status bar          */
#define CP_ERROR        6   /* bright white on red - error messages        */
#define CP_PHRASE       7   /* black on bright white - phrase button       */
#define CP_PHRASE_SEL   8   /* bright white on dark magenta - selected btn */
#define CP_SLIDER       9   /* black on yellow - slider track              */

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */

/** Draw a centred string on the given row using current attributes. */
static void draw_centered(WINDOW *win, int row, const char *text)
{
    int w = getmaxx(win);
    int len = (int)strlen(text);
    int col = (w - len) / 2;
    if (col < 0) col = 0;
    mvwprintw(win, row, col, "%s", text);
}

/** Draw a box with a coloured title bar. */
static void draw_titled_box(WINDOW *win, const char *title)
{
    wattron(win, COLOR_PAIR(CP_BORDER));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(CP_BORDER));

    wattron(win, COLOR_PAIR(CP_TITLE) | A_BOLD);
    draw_centered(win, 0, title);
    wattroff(win, COLOR_PAIR(CP_TITLE) | A_BOLD);
}

/** Draw a status-bar footer centred in the last row of win. */
static void draw_footer(WINDOW *win, const char *text)
{
    int rows = getmaxy(win);
    wattron(win, COLOR_PAIR(CP_STATUS) | A_BOLD);
    mvwhline(win, rows - 1, 1, ' ', getmaxx(win) - 2);
    draw_centered(win, rows - 1, text);
    wattroff(win, COLOR_PAIR(CP_STATUS) | A_BOLD);
}

/* -------------------------------------------------------------------------
 * ui_init
 * ---------------------------------------------------------------------- */
void ui_init(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    use_default_colors();

    /* High-contrast colour scheme */
    init_pair(CP_NORMAL,     COLOR_WHITE,   COLOR_BLUE);
    init_pair(CP_HIGHLIGHT,  COLOR_BLACK,   COLOR_CYAN);
    init_pair(CP_TITLE,      COLOR_YELLOW,  COLOR_BLUE);
    init_pair(CP_BORDER,     COLOR_WHITE,   COLOR_BLUE);
    init_pair(CP_STATUS,     COLOR_BLACK,   COLOR_GREEN);
    init_pair(CP_ERROR,      COLOR_WHITE,   COLOR_RED);
    init_pair(CP_PHRASE,     COLOR_BLACK,   COLOR_WHITE);
    init_pair(CP_PHRASE_SEL, COLOR_WHITE,   COLOR_MAGENTA);
    init_pair(CP_SLIDER,     COLOR_BLACK,   COLOR_YELLOW);

    bkgd(COLOR_PAIR(CP_NORMAL));
    refresh();
}

/* -------------------------------------------------------------------------
 * ui_cleanup
 * ---------------------------------------------------------------------- */
void ui_cleanup(void)
{
    endwin();
}

/* -------------------------------------------------------------------------
 * ui_main_menu
 * ---------------------------------------------------------------------- */
int ui_main_menu(void)
{
    static const char *items[MENU_ITEM_COUNT] = {
        "  1.  Type Message   ",
        "  2.  Quick Phrases  ",
        "  3.  Voice Settings ",
        "  4.  Export WAV     ",
        "  5.  About          ",
        "  6.  Exit           "
    };

    static const char *subtitles[MENU_ITEM_COUNT] = {
        "Speak any text you type",
        "Choose from common AAC phrases",
        "Adjust pitch, speed, volume",
        "Save speech as a WAV file",
        "About OpenVoice",
        "Quit the application"
    };

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    /* Panel dimensions */
    int box_h = MENU_ITEM_COUNT + 10;
    int box_w = 52;
    int box_y = (rows - box_h) / 2;
    int box_x = (cols - box_w) / 2;
    if (box_y < 0) box_y = 0;
    if (box_x < 0) box_x = 0;

    WINDOW *win = newwin(box_h, box_w, box_y, box_x);
    keypad(win, TRUE);
    wbkgd(win, COLOR_PAIR(CP_NORMAL));

    int selection = 0;

    while (1) {
        werase(win);
        draw_titled_box(win, "[ OPENVOICE AAC COMMUNICATOR ]");

        /* Logo / tagline */
        wattron(win, COLOR_PAIR(CP_TITLE) | A_BOLD);
        draw_centered(win, 2, "OpenVoice");
        wattroff(win, COLOR_PAIR(CP_TITLE) | A_BOLD);

        wattron(win, COLOR_PAIR(CP_NORMAL));
        draw_centered(win, 3, "AAC System for Speech Impairments");
        wattroff(win, COLOR_PAIR(CP_NORMAL));

        /* Separator */
        wattron(win, COLOR_PAIR(CP_BORDER));
        mvwhline(win, 4, 1, ACS_HLINE, box_w - 2);
        wattroff(win, COLOR_PAIR(CP_BORDER));

        /* Menu items */
        for (int i = 0; i < MENU_ITEM_COUNT; i++) {
            if (i == selection) {
                wattron(win, COLOR_PAIR(CP_HIGHLIGHT) | A_BOLD);
                mvwprintw(win, 5 + i, 3, "%-44s", items[i]);
                wattroff(win, COLOR_PAIR(CP_HIGHLIGHT) | A_BOLD);
            } else {
                wattron(win, COLOR_PAIR(CP_NORMAL));
                mvwprintw(win, 5 + i, 3, "%-44s", items[i]);
                wattroff(win, COLOR_PAIR(CP_NORMAL));
            }
        }

        /* Separator */
        wattron(win, COLOR_PAIR(CP_BORDER));
        mvwhline(win, 5 + MENU_ITEM_COUNT, 1, ACS_HLINE, box_w - 2);
        wattroff(win, COLOR_PAIR(CP_BORDER));

        /* Subtitle for highlighted item */
        wattron(win, COLOR_PAIR(CP_TITLE));
        mvwprintw(win, 6 + MENU_ITEM_COUNT, 2, "%-48s",
                  subtitles[selection]);
        wattroff(win, COLOR_PAIR(CP_TITLE));

        draw_footer(win, " UP/DOWN: Navigate   ENTER: Select ");

        wrefresh(win);

        int ch = wgetch(win);
        switch (ch) {
            case KEY_UP:
                selection = (selection - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
                break;
            case KEY_DOWN:
                selection = (selection + 1) % MENU_ITEM_COUNT;
                break;
            case '\n':
            case KEY_ENTER:
                delwin(win);
                return selection;
            case '1': delwin(win); return MENU_TYPE_MESSAGE;
            case '2': delwin(win); return MENU_QUICK_PHRASES;
            case '3': delwin(win); return MENU_VOICE_SETTINGS;
            case '4': delwin(win); return MENU_EXPORT_WAV;
            case '5': delwin(win); return MENU_ABOUT;
            case '6':
            case 'q':
            case 'Q':
                delwin(win);
                return MENU_EXIT;
            default:
                break;
        }
    }
}

/* -------------------------------------------------------------------------
 * ui_phrase_board
 * ---------------------------------------------------------------------- */
int ui_phrase_board(const PhraseList *phrases)
{
    if (!phrases || phrases->count == 0)
        return -1;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    /* Up to 4 columns */
    int ncols = 4;
    if (phrases->count < ncols) ncols = phrases->count;
    int btn_w  = 18;
    int btn_h  = 3;
    int pad    = 2;
    int nrows  = (phrases->count + ncols - 1) / ncols;

    int box_w  = ncols * (btn_w + pad) + pad + 2;
    int box_h  = nrows * (btn_h + 1) + 6;
    if (box_h > rows - 2) box_h = rows - 2;
    if (box_w > cols - 2) box_w = cols - 2;

    int box_y  = (rows - box_h) / 2;
    int box_x  = (cols - box_w) / 2;
    if (box_y < 0) box_y = 0;
    if (box_x < 0) box_x = 0;

    WINDOW *win = newwin(box_h, box_w, box_y, box_x);
    keypad(win, TRUE);
    wbkgd(win, COLOR_PAIR(CP_NORMAL));

    int selection = 0;

    while (1) {
        werase(win);
        draw_titled_box(win, "[ QUICK PHRASE BOARD ]");
        draw_footer(win, " Arrows: Navigate   ENTER: Speak   ESC: Back ");

        for (int i = 0; i < phrases->count; i++) {
            int r   = i / ncols;
            int c   = i % ncols;
            int y   = 2 + r * (btn_h + 1);
            int x   = 1 + pad + c * (btn_w + pad);

            if (y + btn_h >= box_h - 1) break; /* out of visible area */

            WINDOW *btn = derwin(win, btn_h, btn_w, y, x);
            if (!btn) continue;

            if (i == selection) {
                wbkgd(btn, COLOR_PAIR(CP_PHRASE_SEL));
                wattron(btn, COLOR_PAIR(CP_PHRASE_SEL) | A_BOLD);
            } else {
                wbkgd(btn, COLOR_PAIR(CP_PHRASE));
                wattron(btn, COLOR_PAIR(CP_PHRASE));
            }

            werase(btn);
            box(btn, 0, 0);

            /* Truncate phrase text to fit button width */
            char label[20];
            snprintf(label, sizeof(label), "%-*.*s",
                     btn_w - 2, btn_w - 2, phrases->items[i].text);
            mvwprintw(btn, 1, 1, "%s", label);

            if (i == selection)
                wattroff(btn, COLOR_PAIR(CP_PHRASE_SEL) | A_BOLD);
            else
                wattroff(btn, COLOR_PAIR(CP_PHRASE));

            wrefresh(btn);
            delwin(btn);
        }

        wrefresh(win);

        int ch = wgetch(win);
        switch (ch) {
            case KEY_LEFT:
                if (selection > 0) selection--;
                break;
            case KEY_RIGHT:
                if (selection < phrases->count - 1) selection++;
                break;
            case KEY_UP:
                if (selection - ncols >= 0) selection -= ncols;
                break;
            case KEY_DOWN:
                if (selection + ncols < phrases->count) selection += ncols;
                break;
            case '\n':
            case KEY_ENTER:
                delwin(win);
                return selection;
            case 27: /* ESC */
                delwin(win);
                return -1;
            default:
                break;
        }
    }
}

/* -------------------------------------------------------------------------
 * ui_voice_settings
 * ---------------------------------------------------------------------- */
void ui_voice_settings(Profile *profile)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int box_h = 18;
    int box_w = 56;
    int box_y = (rows - box_h) / 2;
    int box_x = (cols - box_w) / 2;
    if (box_y < 0) box_y = 0;
    if (box_x < 0) box_x = 0;

    WINDOW *win = newwin(box_h, box_w, box_y, box_x);
    keypad(win, TRUE);
    wbkgd(win, COLOR_PAIR(CP_NORMAL));

    /* Parameters: label, pointer, min, max, step */
    typedef struct { const char *label; int *val; int lo; int hi; int step; } Param;
    Param params[3] = {
        { "Pitch  ", &profile->pitch,  PITCH_MIN,  PITCH_MAX,  5  },
        { "Speed  ", &profile->speed,  SPEED_MIN,  SPEED_MAX,  10 },
        { "Volume ", &profile->volume, VOLUME_MIN, VOLUME_MAX, 10 }
    };
    int nparams = 3;
    int sel     = 0;

    while (1) {
        werase(win);
        draw_titled_box(win, "[ VOICE SETTINGS ]");
        draw_footer(win, " UP/DN: Select  LEFT/RIGHT: Adjust  S: Save  ESC: Back ");

        for (int i = 0; i < nparams; i++) {
            int y = 3 + i * 4;

            /* Parameter label */
            if (i == sel)
                wattron(win, COLOR_PAIR(CP_HIGHLIGHT) | A_BOLD);
            else
                wattron(win, COLOR_PAIR(CP_TITLE) | A_BOLD);

            mvwprintw(win, y, 3, "%s", params[i].label);

            if (i == sel)
                wattroff(win, COLOR_PAIR(CP_HIGHLIGHT) | A_BOLD);
            else
                wattroff(win, COLOR_PAIR(CP_TITLE) | A_BOLD);

            /* Numeric value */
            wattron(win, COLOR_PAIR(CP_NORMAL) | A_BOLD);
            mvwprintw(win, y, 12, "%3d", *params[i].val);
            wattroff(win, COLOR_PAIR(CP_NORMAL) | A_BOLD);

            /* Slider bar */
            int bar_w = box_w - 20;
            int filled = (int)(((double)(*params[i].val - params[i].lo) /
                                (params[i].hi - params[i].lo)) * bar_w);

            mvwprintw(win, y + 1, 3, "[");
            for (int j = 0; j < bar_w; j++) {
                if (j < filled) {
                    wattron(win, COLOR_PAIR(CP_SLIDER) | A_BOLD);
                    waddch(win, '=');
                    wattroff(win, COLOR_PAIR(CP_SLIDER) | A_BOLD);
                } else {
                    wattron(win, COLOR_PAIR(CP_NORMAL));
                    waddch(win, '-');
                    wattroff(win, COLOR_PAIR(CP_NORMAL));
                }
            }
            mvwprintw(win, y + 1, 3 + bar_w + 1, "]");
        }

        /* Language display */
        wattron(win, COLOR_PAIR(CP_TITLE) | A_BOLD);
        mvwprintw(win, 3 + nparams * 4 + 1, 3, "Language: %s", profile->language);
        wattroff(win, COLOR_PAIR(CP_TITLE) | A_BOLD);

        wrefresh(win);

        int ch = wgetch(win);
        switch (ch) {
            case KEY_UP:
                sel = (sel - 1 + nparams) % nparams;
                break;
            case KEY_DOWN:
                sel = (sel + 1) % nparams;
                break;
            case KEY_RIGHT:
                *params[sel].val += params[sel].step;
                if (*params[sel].val > params[sel].hi)
                    *params[sel].val = params[sel].hi;
                break;
            case KEY_LEFT:
                *params[sel].val -= params[sel].step;
                if (*params[sel].val < params[sel].lo)
                    *params[sel].val = params[sel].lo;
                break;
            case 's':
            case 'S':
                /* Save handled by caller after return */
                delwin(win);
                return;
            case 27: /* ESC */
                delwin(win);
                return;
            default:
                break;
        }
    }
}

/* -------------------------------------------------------------------------
 * ui_get_text_input
 * ---------------------------------------------------------------------- */
int ui_get_text_input(const char *prompt, char *buf, int maxlen)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int box_w = 60;
    int box_h = 7;
    int box_y = (rows - box_h) / 2;
    int box_x = (cols - box_w) / 2;
    if (box_y < 0) box_y = 0;
    if (box_x < 0) box_x = 0;

    WINDOW *win = newwin(box_h, box_w, box_y, box_x);
    keypad(win, TRUE);
    wbkgd(win, COLOR_PAIR(CP_NORMAL));
    curs_set(1);

    int pos = 0;       /* cursor position in buf */
    int len = 0;       /* current string length  */
    buf[0]  = '\0';

    /* Input field dimensions */
    int field_w = box_w - 4;
    int field_x = 2;
    int field_y = 4;

    while (1) {
        werase(win);
        draw_titled_box(win, "[ INPUT ]");
        draw_footer(win, " ENTER: Confirm   ESC: Cancel ");

        wattron(win, COLOR_PAIR(CP_TITLE) | A_BOLD);
        mvwprintw(win, 2, 2, "%s", prompt);
        wattroff(win, COLOR_PAIR(CP_TITLE) | A_BOLD);

        /* Input field background */
        wattron(win, COLOR_PAIR(CP_PHRASE));
        for (int i = 0; i < field_w; i++)
            mvwaddch(win, field_y, field_x + i, ' ');
        wattroff(win, COLOR_PAIR(CP_PHRASE));

        /* Display text (scroll left if needed) */
        int disp_start = 0;
        if (pos >= field_w) disp_start = pos - field_w + 1;

        wattron(win, COLOR_PAIR(CP_PHRASE) | A_BOLD);
        for (int i = 0; i < field_w && disp_start + i < len; i++) {
            mvwaddch(win, field_y, field_x + i,
                     (unsigned char)buf[disp_start + i]);
        }
        wattroff(win, COLOR_PAIR(CP_PHRASE) | A_BOLD);

        /* Move hardware cursor */
        int cx = pos - disp_start;
        if (cx < 0) cx = 0;
        if (cx >= field_w) cx = field_w - 1;
        wmove(win, field_y, field_x + cx);

        wrefresh(win);

        int ch = wgetch(win);
        switch (ch) {
            case '\n':
            case KEY_ENTER:
                buf[len] = '\0';
                curs_set(0);
                delwin(win);
                return UI_OK;

            case 27: /* ESC */
                buf[0] = '\0';
                curs_set(0);
                delwin(win);
                return UI_CANCEL;

            case KEY_BACKSPACE:
            case 127:
            case '\b':
                if (pos > 0) {
                    memmove(buf + pos - 1, buf + pos, (size_t)(len - pos));
                    pos--;
                    len--;
                    buf[len] = '\0';
                }
                break;

            case KEY_DC: /* Delete */
                if (pos < len) {
                    memmove(buf + pos, buf + pos + 1, (size_t)(len - pos - 1));
                    len--;
                    buf[len] = '\0';
                }
                break;

            case KEY_LEFT:
                if (pos > 0) pos--;
                break;

            case KEY_RIGHT:
                if (pos < len) pos++;
                break;

            case KEY_HOME:
                pos = 0;
                break;

            case KEY_END:
                pos = len;
                break;

            default:
                if (ch >= 32 && ch < 256 && len < maxlen - 1) {
                    memmove(buf + pos + 1, buf + pos, (size_t)(len - pos));
                    buf[pos] = (char)ch;
                    pos++;
                    len++;
                    buf[len] = '\0';
                }
                break;
        }
    }
}

/* -------------------------------------------------------------------------
 * ui_show_message
 * ---------------------------------------------------------------------- */
void ui_show_message(const char *title, const char *message)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int box_w = 50;
    int box_h = 7;
    int box_y = (rows - box_h) / 2;
    int box_x = (cols - box_w) / 2;
    if (box_y < 0) box_y = 0;
    if (box_x < 0) box_x = 0;

    WINDOW *win = newwin(box_h, box_w, box_y, box_x);
    keypad(win, TRUE);
    wbkgd(win, COLOR_PAIR(CP_NORMAL));

    draw_titled_box(win, title);
    wattron(win, COLOR_PAIR(CP_NORMAL));
    draw_centered(win, 3, message);
    wattroff(win, COLOR_PAIR(CP_NORMAL));
    draw_footer(win, " Press any key to continue ");
    wrefresh(win);

    wgetch(win);
    delwin(win);
}

/* -------------------------------------------------------------------------
 * ui_show_info_screen
 * ---------------------------------------------------------------------- */
void ui_show_info_screen(const char *title, const char **lines)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    /* Count lines */
    int nlines = 0;
    while (lines[nlines]) nlines++;

    int box_h = nlines + 5;
    int box_w = 56;
    if (box_h > rows - 2) box_h = rows - 2;
    int box_y = (rows - box_h) / 2;
    int box_x = (cols - box_w) / 2;
    if (box_y < 0) box_y = 0;
    if (box_x < 0) box_x = 0;

    WINDOW *win = newwin(box_h, box_w, box_y, box_x);
    keypad(win, TRUE);
    wbkgd(win, COLOR_PAIR(CP_NORMAL));

    draw_titled_box(win, title);
    draw_footer(win, " Press any key to return ");

    for (int i = 0; i < nlines && 2 + i < box_h - 2; i++) {
        wattron(win, COLOR_PAIR(CP_NORMAL));
        mvwprintw(win, 2 + i, 3, "%-*.*s",
                  box_w - 4, box_w - 4, lines[i]);
        wattroff(win, COLOR_PAIR(CP_NORMAL));
    }

    wrefresh(win);
    wgetch(win);
    delwin(win);
}
