/*
 * OpenVoice AAC Communicator
 * src/ui.c - Windows Console User Interface
 *
 * Uses the Windows Console API directly so the binary runs on any
 * Windows machine without additional installs.
 *
 * Standard: C99 + Windows SDK
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../include/ui.h"
#include "../include/phrase.h"
#include "../include/profile.h"

/* ---- console handles --------------------------------------------------- */
static HANDLE hOut;
static HANDLE hIn;
static DWORD  g_orig_in_mode;
static WORD   g_orig_attr;

/* ---- colour attributes -------------------------------------------------- */
#define ATTR_NORMAL    (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE \
                        | BACKGROUND_BLUE)
#define ATTR_HIGHLIGHT (FOREGROUND_INTENSITY \
                        | BACKGROUND_GREEN|BACKGROUND_BLUE)
#define ATTR_TITLE     (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY \
                        | BACKGROUND_BLUE)
#define ATTR_BORDER    (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE \
                        | FOREGROUND_INTENSITY | BACKGROUND_BLUE)
#define ATTR_STATUS    (BACKGROUND_GREEN | FOREGROUND_INTENSITY)
#define ATTR_PHRASE    (BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE \
                        | BACKGROUND_INTENSITY)
#define ATTR_PHRASE_SEL (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE \
                         | FOREGROUND_INTENSITY \
                         | BACKGROUND_RED|BACKGROUND_BLUE)
#define ATTR_SLIDER    (BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_INTENSITY)

/* ---- logical key codes (above ASCII range) ------------------------------ */
#define K_UP    0x0100
#define K_DOWN  0x0101
#define K_LEFT  0x0102
#define K_RIGHT 0x0103
#define K_ENTER 0x0104
#define K_ESC   0x0105
#define K_BKSP  0x0106
#define K_DEL   0x0107
#define K_HOME  0x0108
#define K_END   0x0109

/* =========================================================================
 * Low-level console primitives
 * ====================================================================== */

static void con_get_size(int *w, int *h)
{
    CONSOLE_SCREEN_BUFFER_INFO i;
    GetConsoleScreenBufferInfo(hOut, &i);
    *w = i.srWindow.Right  - i.srWindow.Left + 1;
    *h = i.srWindow.Bottom - i.srWindow.Top  + 1;
}

static void con_move(int x, int y)
{
    COORD c = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hOut, c);
}

static void con_attr(WORD a)
{
    SetConsoleTextAttribute(hOut, a);
}

static void con_str(int x, int y, WORD a, const char *s)
{
    con_move(x, y);
    con_attr(a);
    DWORD wr;
    WriteConsoleA(hOut, s, (DWORD)strlen(s), &wr, NULL);
}

static void con_printf(int x, int y, WORD a, const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    con_str(x, y, a, buf);
}

static void con_fill(int x, int y, int w, WORD a, char ch)
{
    if (w <= 0) return;
    COORD pos = { (SHORT)x, (SHORT)y };
    DWORD wr;
    FillConsoleOutputAttribute(hOut, a, (DWORD)w, pos, &wr);
    FillConsoleOutputCharacterA(hOut, ch, (DWORD)w, pos, &wr);
}

static void con_clear_rect(int x, int y, int w, int h, WORD a)
{
    for (int r = 0; r < h; r++)
        con_fill(x, y + r, w, a, ' ');
}

static void con_cursor(int visible)
{
    CONSOLE_CURSOR_INFO ci;
    GetConsoleCursorInfo(hOut, &ci);
    ci.bVisible = visible ? TRUE : FALSE;
    SetConsoleCursorInfo(hOut, &ci);
}

/* =========================================================================
 * Drawing helpers
 * ====================================================================== */

static void draw_box(int x, int y, int w, int h, WORD a)
{
    /* top */
    con_fill(x, y, w, a, '-');
    con_str(x,     y, a, "+");
    con_str(x+w-1, y, a, "+");
    /* bottom */
    con_fill(x, y+h-1, w, a, '-');
    con_str(x,     y+h-1, a, "+");
    con_str(x+w-1, y+h-1, a, "+");
    /* sides */
    for (int r = y+1; r < y+h-1; r++) {
        con_str(x,     r, a, "|");
        con_str(x+w-1, r, a, "|");
    }
}

static void draw_titled_box(int x, int y, int w, int h, const char *title)
{
    con_clear_rect(x, y, w, h, ATTR_NORMAL);
    draw_box(x, y, w, h, ATTR_BORDER);
    int tlen = (int)strlen(title);
    int tx = x + (w - tlen) / 2;
    if (tx < x+1) tx = x+1;
    con_str(tx, y, ATTR_TITLE, title);
}

static void draw_footer(int x, int y, int w, const char *text)
{
    con_fill(x+1, y, w-2, ATTR_STATUS, ' ');
    int tlen = (int)strlen(text);
    int tx = x + (w - tlen) / 2;
    if (tx < x+1) tx = x+1;
    con_str(tx, y, ATTR_STATUS, text);
}

/* Draw text centred within a box starting at (box_x, row) of width box_w. */
static void draw_centered(int box_x, int row, int box_w, WORD a,
                           const char *text)
{
    int tlen = (int)strlen(text);
    int tx = box_x + (box_w - tlen) / 2;
    if (tx < box_x) tx = box_x;
    con_str(tx, row, a, text);
}

/* =========================================================================
 * Input
 * ====================================================================== */

static int read_key(void)
{
    INPUT_RECORD ir;
    DWORD read;
    while (1) {
        ReadConsoleInputA(hIn, &ir, 1, &read);
        if (ir.EventType != KEY_EVENT || !ir.Event.KeyEvent.bKeyDown)
            continue;
        WORD vk = ir.Event.KeyEvent.wVirtualKeyCode;
        char ch = ir.Event.KeyEvent.uChar.AsciiChar;
        switch (vk) {
            case VK_UP:     return K_UP;
            case VK_DOWN:   return K_DOWN;
            case VK_LEFT:   return K_LEFT;
            case VK_RIGHT:  return K_RIGHT;
            case VK_RETURN: return K_ENTER;
            case VK_ESCAPE: return K_ESC;
            case VK_BACK:   return K_BKSP;
            case VK_DELETE: return K_DEL;
            case VK_HOME:   return K_HOME;
            case VK_END:    return K_END;
            default:
                if (ch >= 32 && (unsigned char)ch < 127)
                    return (int)(unsigned char)ch;
                break;
        }
    }
}

/* =========================================================================
 * Public API
 * ====================================================================== */

void ui_init(void)
{
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    hIn  = GetStdHandle(STD_INPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    g_orig_attr = csbi.wAttributes;

    GetConsoleMode(hIn, &g_orig_in_mode);
    SetConsoleMode(hIn, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    SetConsoleTitleA("OpenVoice AAC Communicator");
    con_cursor(0);

    int w, h;
    con_get_size(&w, &h);
    con_clear_rect(0, 0, w, h, ATTR_NORMAL);
}

void ui_cleanup(void)
{
    int w, h;
    con_get_size(&w, &h);
    con_clear_rect(0, 0, w, h, g_orig_attr);
    con_attr(g_orig_attr);
    con_move(0, 0);
    con_cursor(1);
    SetConsoleMode(hIn, g_orig_in_mode);
}

/* =========================================================================
 * ui_main_menu
 * ====================================================================== */

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

    int cw, ch;
    con_get_size(&cw, &ch);

    int box_h = MENU_ITEM_COUNT + 10;
    int box_w = 52;
    int box_y = (ch - box_h) / 2; if (box_y < 0) box_y = 0;
    int box_x = (cw - box_w) / 2; if (box_x < 0) box_x = 0;

    int selection = 0;
    int dirty = 1;

    while (1) {
        if (dirty) {
            draw_titled_box(box_x, box_y, box_w, box_h,
                            "[ OPENVOICE AAC COMMUNICATOR ]");

            draw_centered(box_x, box_y+2, box_w,
                          ATTR_TITLE | FOREGROUND_INTENSITY, "OpenVoice");
            draw_centered(box_x, box_y+3, box_w,
                          ATTR_NORMAL, "AAC System for Speech Impairments");
            con_fill(box_x+1, box_y+4, box_w-2, ATTR_BORDER, '-');

            for (int i = 0; i < MENU_ITEM_COUNT; i++) {
                WORD a = (i == selection) ? ATTR_HIGHLIGHT : ATTR_NORMAL;
                char padded[50];
                snprintf(padded, sizeof(padded), "%-44s", items[i]);
                con_str(box_x+3, box_y+5+i, a, padded);
            }

            con_fill(box_x+1, box_y+5+MENU_ITEM_COUNT, box_w-2, ATTR_BORDER, '-');

            char sub[52];
            snprintf(sub, sizeof(sub), "%-48s", subtitles[selection]);
            con_str(box_x+2, box_y+6+MENU_ITEM_COUNT, ATTR_TITLE, sub);

            draw_footer(box_x, box_y+box_h-1, box_w,
                        " UP/DOWN: Navigate   ENTER: Select ");
            dirty = 0;
        }

        int key = read_key();
        switch (key) {
            case K_UP:
                selection = (selection - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
                dirty = 1;
                break;
            case K_DOWN:
                selection = (selection + 1) % MENU_ITEM_COUNT;
                dirty = 1;
                break;
            case K_ENTER:
                return selection;
            case '1': return MENU_TYPE_MESSAGE;
            case '2': return MENU_QUICK_PHRASES;
            case '3': return MENU_VOICE_SETTINGS;
            case '4': return MENU_EXPORT_WAV;
            case '5': return MENU_ABOUT;
            case '6':
            case 'q':
            case 'Q':
            case K_ESC:
                return MENU_EXIT;
            default:
                break;
        }
    }
}

/* =========================================================================
 * ui_phrase_board
 * ====================================================================== */

int ui_phrase_board(const PhraseList *phrases)
{
    if (!phrases || phrases->count == 0) return -1;

    int cw, ch;
    con_get_size(&cw, &ch);

    int ncols = 4;
    if (phrases->count < ncols) ncols = phrases->count;
    int btn_w = 20;   /* wide enough for "I Don't Understand" (18 chars) */
    int btn_h = 3;
    int pad   = 2;
    int nrows = (phrases->count + ncols - 1) / ncols;

    int box_w = ncols * (btn_w + pad) + pad + 2;
    int box_h = nrows * (btn_h + 1) + 6;
    if (box_h > ch - 2) box_h = ch - 2;
    if (box_w > cw - 2) box_w = cw - 2;

    int box_y = (ch - box_h) / 2; if (box_y < 0) box_y = 0;
    int box_x = (cw - box_w) / 2; if (box_x < 0) box_x = 0;

    int selection = 0;
    int dirty = 1;

    while (1) {
        if (dirty) {
            draw_titled_box(box_x, box_y, box_w, box_h,
                            "[ QUICK PHRASE BOARD ]");
            draw_footer(box_x, box_y+box_h-1, box_w,
                        " Arrows: Navigate   ENTER: Speak   ESC: Back ");

            for (int i = 0; i < phrases->count; i++) {
                int r  = i / ncols;
                int c  = i % ncols;
                int bx = box_x + 1 + pad + c * (btn_w + pad);
                int by = box_y + 2 + r * (btn_h + 1);

                if (by + btn_h >= box_y + box_h - 1) break;

                WORD a = (i == selection) ? ATTR_PHRASE_SEL : ATTR_PHRASE;

                /* button outline */
                con_fill(bx,       by,   btn_w, a, '-');
                con_str(bx,        by,   a, "+");
                con_str(bx+btn_w-1,by,   a, "+");
                con_fill(bx,       by+1, btn_w, a, ' ');
                con_str(bx,        by+1, a, "|");
                con_str(bx+btn_w-1,by+1, a, "|");
                con_fill(bx,       by+2, btn_w, a, '-');
                con_str(bx,        by+2, a, "+");
                con_str(bx+btn_w-1,by+2, a, "+");

                /* phrase text */
                char label[22];
                snprintf(label, sizeof(label), "%-*.*s",
                         btn_w-2, btn_w-2, phrases->items[i].text);
                con_str(bx+1, by+1, a, label);
            }
            dirty = 0;
        }

        int key = read_key();
        switch (key) {
            case K_LEFT:
                if (selection > 0) { selection--; dirty = 1; }
                break;
            case K_RIGHT:
                if (selection < phrases->count - 1) { selection++; dirty = 1; }
                break;
            case K_UP:
                if (selection - ncols >= 0) { selection -= ncols; dirty = 1; }
                break;
            case K_DOWN:
                if (selection + ncols < phrases->count) { selection += ncols; dirty = 1; }
                break;
            case K_ENTER:
                return selection;
            case K_ESC:
                return -1;
            default:
                break;
        }
    }
}

/* =========================================================================
 * ui_voice_settings
 * ====================================================================== */

void ui_voice_settings(Profile *profile)
{
    int cw, ch;
    con_get_size(&cw, &ch);

    int box_h = 18;
    int box_w = 60;   /* 60 gives 58-char interior — fits the 55-char footer */
    int box_y = (ch - box_h) / 2; if (box_y < 0) box_y = 0;
    int box_x = (cw - box_w) / 2; if (box_x < 0) box_x = 0;

    /* Snapshot so ESC can restore original values */
    Profile backup = *profile;

    typedef struct {
        const char *label; int *val; int lo; int hi; int step;
    } Param;

    Param params[3] = {
        { "Pitch(st)", &profile->pitch,  PITCH_MIN,  PITCH_MAX,  1  },
        { "Speed    ", &profile->speed,  SPEED_MIN,  SPEED_MAX,  10 },
        { "Volume   ", &profile->volume, VOLUME_MIN, VOLUME_MAX, 10 }
    };
    int nparams = 3;
    int sel   = 0;
    int dirty = 1;

    while (1) {
        if (dirty) {
            draw_titled_box(box_x, box_y, box_w, box_h, "[ VOICE SETTINGS ]");
            draw_footer(box_x, box_y+box_h-1, box_w,
                        " UP/DN: Select  LEFT/RIGHT: Adjust  S: Save  ESC: Back ");

            int bar_w = box_w - 20;

            for (int i = 0; i < nparams; i++) {
                int y = box_y + 3 + i * 4;

                WORD la = (i == sel)
                          ? ATTR_HIGHLIGHT
                          : (ATTR_TITLE | FOREGROUND_INTENSITY);
                con_printf(box_x+3, y, la, "%s", params[i].label);
                con_printf(box_x+12, y, ATTR_NORMAL, "%3d", *params[i].val);

                int filled = (int)(
                    ((double)(*params[i].val - params[i].lo)
                    / (params[i].hi - params[i].lo)) * bar_w);

                con_str(box_x+3, y+1, ATTR_NORMAL, "[");
                if (filled > 0)
                    con_fill(box_x+4, y+1, filled, ATTR_SLIDER, '=');
                if (filled < bar_w)
                    con_fill(box_x+4+filled, y+1, bar_w-filled, ATTR_NORMAL, '-');
                con_str(box_x+4+bar_w, y+1, ATTR_NORMAL, "]");

                /* Scale hint under the pitch slider only */
                if (i == 0) {
                    con_printf(box_x+3, y+2, ATTR_TITLE,
                               "  -10st <------ F0 pitch ------> +10st  ");
                }
            }

            con_printf(box_x+3, box_y+3+nparams*4+1, ATTR_TITLE,
                       "Language: %s", profile->language);

            dirty = 0;
        }

        int key = read_key();
        switch (key) {
            case K_UP:
                sel = (sel - 1 + nparams) % nparams; dirty = 1; break;
            case K_DOWN:
                sel = (sel + 1) % nparams; dirty = 1; break;
            case K_RIGHT:
                *params[sel].val += params[sel].step;
                if (*params[sel].val > params[sel].hi)
                    *params[sel].val = params[sel].hi;
                dirty = 1;
                break;
            case K_LEFT:
                *params[sel].val -= params[sel].step;
                if (*params[sel].val < params[sel].lo)
                    *params[sel].val = params[sel].lo;
                dirty = 1;
                break;
            case 's':
            case 'S':
                ui_show_message("[ SAVED ]",
                    "Voice settings saved successfully.");
                return;
            case K_ESC:
                *profile = backup;   /* discard changes */
                return;
            default:
                break;
        }
    }
}

/* =========================================================================
 * ui_get_text_input
 * ====================================================================== */

int ui_get_text_input(const char *prompt, char *buf, int maxlen)
{
    int cw, ch;
    con_get_size(&cw, &ch);

    int box_w  = 60;
    int box_h  = 7;
    int box_y  = (ch - box_h) / 2; if (box_y < 0) box_y = 0;
    int box_x  = (cw - box_w) / 2; if (box_x < 0) box_x = 0;

    int field_w = box_w - 4;
    int field_x = box_x + 2;
    int field_y = box_y + 4;

    int pos = 0;
    int len = 0;
    buf[0]  = '\0';

    con_cursor(1);

    while (1) {
        draw_titled_box(box_x, box_y, box_w, box_h, "[ INPUT ]");
        draw_footer(box_x, box_y+box_h-1, box_w, " ENTER: Confirm   ESC: Cancel ");
        con_str(box_x+2, box_y+2, ATTR_TITLE, prompt);

        /* field background */
        con_fill(field_x, field_y, field_w, ATTR_PHRASE, ' ');

        /* visible text (scroll if needed) */
        int disp_start = 0;
        if (pos >= field_w) disp_start = pos - field_w + 1;

        for (int i = 0; i < field_w && disp_start + i < len; i++)
            con_fill(field_x + i, field_y, 1, ATTR_PHRASE,
                     buf[disp_start + i]);

        /* place hardware cursor */
        int cx = pos - disp_start;
        if (cx < 0)        cx = 0;
        if (cx >= field_w) cx = field_w - 1;
        con_move(field_x + cx, field_y);
        con_attr(ATTR_PHRASE);

        int key = read_key();
        switch (key) {
            case K_ENTER:
                buf[len] = '\0';
                con_cursor(0);
                return UI_OK;

            case K_ESC:
                buf[0] = '\0';
                con_cursor(0);
                return UI_CANCEL;

            case K_BKSP:
                if (pos > 0) {
                    memmove(buf + pos - 1, buf + pos,
                            (size_t)(len - pos));
                    pos--; len--;
                    buf[len] = '\0';
                }
                break;

            case K_DEL:
                if (pos < len) {
                    memmove(buf + pos, buf + pos + 1,
                            (size_t)(len - pos - 1));
                    len--;
                    buf[len] = '\0';
                }
                break;

            case K_LEFT:  if (pos > 0)   pos--; break;
            case K_RIGHT: if (pos < len) pos++; break;
            case K_HOME:  pos = 0;   break;
            case K_END:   pos = len; break;

            default:
                if (key >= 32 && key < 256 && len < maxlen - 1) {
                    memmove(buf + pos + 1, buf + pos,
                            (size_t)(len - pos));
                    buf[pos] = (char)key;
                    pos++; len++;
                    buf[len] = '\0';
                }
                break;
        }
    }
}

/* =========================================================================
 * ui_speak_status
 *
 * Draws a non-blocking "Speaking…" overlay so the user knows audio is
 * being synthesised.  Returns immediately; the caller then invokes
 * speak_text() which blocks until playback finishes.
 * ====================================================================== */

void ui_speak_status(const char *text)
{
    int cw, ch;
    con_get_size(&cw, &ch);

    int box_w = 56;
    int box_h = 6;
    int box_y = (ch - box_h) / 2; if (box_y < 0) box_y = 0;
    int box_x = (cw - box_w) / 2; if (box_x < 0) box_x = 0;

    draw_titled_box(box_x, box_y, box_w, box_h, "[ SPEAKING ]");

    /* Truncate long messages to fit inside the box */
    char label[53];
    snprintf(label, sizeof(label), "%-*.*s", box_w - 4, box_w - 4, text);

    draw_centered(box_x, box_y + 2, box_w, ATTR_TITLE, label);
    draw_footer(box_x, box_y + box_h - 1, box_w,
                " Please wait \x18\x19 Synthesising audio... ");
}

/* =========================================================================
 * ui_show_message
 * ====================================================================== */

void ui_show_message(const char *title, const char *message)
{
    int cw, ch;
    con_get_size(&cw, &ch);

    int box_w = 50;
    int box_h = 7;
    int box_y = (ch - box_h) / 2; if (box_y < 0) box_y = 0;
    int box_x = (cw - box_w) / 2; if (box_x < 0) box_x = 0;

    draw_titled_box(box_x, box_y, box_w, box_h, title);
    draw_centered(box_x, box_y+3, box_w, ATTR_NORMAL, message);
    draw_footer(box_x, box_y+box_h-1, box_w, " Press any key to continue ");

    read_key();
}

/* =========================================================================
 * ui_show_info_screen
 * ====================================================================== */

void ui_show_info_screen(const char *title, const char **lines)
{
    int cw, ch;
    con_get_size(&cw, &ch);

    int nlines = 0;
    while (lines[nlines]) nlines++;

    int box_h = nlines + 5;
    int box_w = 56;
    if (box_h > ch - 2) box_h = ch - 2;
    int box_y = (ch - box_h) / 2; if (box_y < 0) box_y = 0;
    int box_x = (cw - box_w) / 2; if (box_x < 0) box_x = 0;

    draw_titled_box(box_x, box_y, box_w, box_h, title);
    draw_footer(box_x, box_y+box_h-1, box_w, " Press any key to return ");

    for (int i = 0; i < nlines && 2 + i < box_h - 2; i++) {
        char padded[64];
        snprintf(padded, sizeof(padded), "%-*.*s", box_w-4, box_w-4, lines[i]);
        con_str(box_x+3, box_y+2+i, ATTR_NORMAL, padded);
    }

    read_key();
}

