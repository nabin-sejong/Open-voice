/*
 * OpenVoice AAC Communicator
 * include/ui.h - ncurses User Interface API
 *
 * Declares all functions and constants used by the terminal UI layer.
 *
 * Standard: C99
 */

#ifndef OPENVOICE_UI_H
#define OPENVOICE_UI_H

#include "phrase.h"
#include "profile.h"

/* -------------------------------------------------------------------------
 * Return codes
 * ---------------------------------------------------------------------- */
#define UI_OK      0
#define UI_CANCEL  1
#define UI_ERROR  -1

/* -------------------------------------------------------------------------
 * Main-menu item indices
 * ---------------------------------------------------------------------- */
#define MENU_TYPE_MESSAGE    0
#define MENU_QUICK_PHRASES   1
#define MENU_VOICE_SETTINGS  2
#define MENU_EXPORT_WAV      3
#define MENU_ABOUT           4
#define MENU_EXIT            5
#define MENU_ITEM_COUNT      6

/* -------------------------------------------------------------------------
 * Layout constants
 * ---------------------------------------------------------------------- */
#define MAX_MESSAGE_LEN   512   /* maximum user-typed message length      */

/* -------------------------------------------------------------------------
 * Lifecycle
 * ---------------------------------------------------------------------- */

/**
 * ui_init - initialise ncurses, colours, and key mappings.
 * Must be called once before any other ui_* function.
 */
void ui_init(void);

/**
 * ui_cleanup - restore the terminal to its original state.
 * Must be called before the process exits.
 */
void ui_cleanup(void);

/* -------------------------------------------------------------------------
 * Screens
 * ---------------------------------------------------------------------- */

/**
 * ui_main_menu - draw the main menu and block until the user selects an item.
 *
 * Returns one of the MENU_* constants defined above.
 */
int ui_main_menu(void);

/**
 * ui_phrase_board - display a grid of quick-phrase buttons.
 *
 * @param phrases  Loaded phrase list to display.
 * Returns the index of the selected phrase, or -1 if cancelled.
 */
int ui_phrase_board(const PhraseList *phrases);

/**
 * ui_voice_settings - interactive editor for pitch, speed, and volume.
 *
 * @param profile  Profile to read from and write to.
 */
void ui_voice_settings(Profile *profile);

/**
 * ui_get_text_input - show a labelled text-entry box.
 *
 * @param prompt   Label displayed above the input field.
 * @param buf      Destination buffer for the entered text.
 * @param maxlen   Size of buf (including NUL terminator).
 * Returns UI_OK on Enter, UI_CANCEL on Escape.
 */
int ui_get_text_input(const char *prompt, char *buf, int maxlen);

/**
 * ui_show_message - display a modal dialogue with a title and single message.
 *
 * @param title    Title bar text.
 * @param message  Body text.
 */
void ui_show_message(const char *title, const char *message);

/**
 * ui_show_info_screen - display a full-screen informational page.
 *
 * @param title  Page title.
 * @param lines  NULL-terminated array of strings to display.
 */
void ui_show_info_screen(const char *title, const char **lines);

#endif /* OPENVOICE_UI_H */
