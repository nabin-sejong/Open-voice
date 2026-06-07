/*
 * OpenVoice AAC Communicator
 * main.c - Application Entry Point
 *
 * A terminal-based Augmentative and Alternative Communication (AAC) system
 * designed for users with speech impairments.
 *
 * Author: OpenVoice Project
 * Standard: C99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "../include/ui.h"
#include "../include/speech.h"
#include "../include/phrase.h"
#include "../include/profile.h"
#include "../include/export.h"

/* -------------------------------------------------------------------------
 * Forward declarations for menu handlers
 * ---------------------------------------------------------------------- */
static void handle_type_message(Profile *profile);
static void handle_quick_phrases(Profile *profile);
static void handle_voice_settings(Profile *profile);
static void handle_export_wav(Profile *profile);
static void handle_about(void);

/* -------------------------------------------------------------------------
 * main() - initialise subsystems and run the menu loop
 * ---------------------------------------------------------------------- */
int main(void)
{
    /* Use the locale so ncurses handles UTF-8 properly */
    setlocale(LC_ALL, "");

    /* Load (or create) the user voice profile */
    Profile profile;
    profile_init(&profile);
    if (profile_load(&profile, PROFILE_PATH) != 0) {
        /* Non-fatal: use defaults if profile file is missing */
        profile_set_defaults(&profile);
    }

    /* Initialise the ncurses UI layer */
    ui_init();

    int choice = 0;
    int running = 1;

    while (running) {
        choice = ui_main_menu();

        switch (choice) {
            case MENU_TYPE_MESSAGE:
                handle_type_message(&profile);
                break;

            case MENU_QUICK_PHRASES:
                handle_quick_phrases(&profile);
                break;

            case MENU_VOICE_SETTINGS:
                handle_voice_settings(&profile);
                break;

            case MENU_EXPORT_WAV:
                handle_export_wav(&profile);
                break;

            case MENU_ABOUT:
                handle_about();
                break;

            case MENU_EXIT:
                running = 0;
                break;

            default:
                /* Ignore unknown selections */
                break;
        }
    }

    /* Tear down ncurses before exit */
    ui_cleanup();

    /* Persist any profile changes made during the session */
    profile_save(&profile, PROFILE_PATH);

    return EXIT_SUCCESS;
}

/* -------------------------------------------------------------------------
 * handle_type_message - let the user type arbitrary text and speak it
 * ---------------------------------------------------------------------- */
static void handle_type_message(Profile *profile)
{
    char message[MAX_MESSAGE_LEN];
    memset(message, 0, sizeof(message));

    if (ui_get_text_input("Enter Message:", message, MAX_MESSAGE_LEN) == UI_OK) {
        if (strlen(message) > 0) {
            speak_text(message, profile);
        }
    }
}

/* -------------------------------------------------------------------------
 * handle_quick_phrases - display phrase board and speak selected phrase
 * ---------------------------------------------------------------------- */
static void handle_quick_phrases(Profile *profile)
{
    PhraseList phrases;
    phrase_list_init(&phrases);

    if (phrase_load(&phrases, PHRASES_PATH) != 0) {
        /* Fall back to built-in defaults if file is unavailable */
        phrase_load_defaults(&phrases);
    }

    int selection = ui_phrase_board(&phrases);

    if (selection >= 0 && selection < phrases.count) {
        speak_text(phrases.items[selection].text, profile);
    }

    phrase_list_free(&phrases);
}

/* -------------------------------------------------------------------------
 * handle_voice_settings - edit pitch / speed / volume and save profile
 * ---------------------------------------------------------------------- */
static void handle_voice_settings(Profile *profile)
{
    ui_voice_settings(profile);
    profile_save(profile, PROFILE_PATH);
}

/* -------------------------------------------------------------------------
 * handle_export_wav - type a message and export it to a WAV file
 * ---------------------------------------------------------------------- */
static void handle_export_wav(Profile *profile)
{
    char message[MAX_MESSAGE_LEN];
    memset(message, 0, sizeof(message));

    if (ui_get_text_input("Enter text to export:", message, MAX_MESSAGE_LEN) == UI_OK) {
        if (strlen(message) > 0) {
            char wav_path[256];
            snprintf(wav_path, sizeof(wav_path), "output.wav");
            if (export_wav(message, wav_path, profile) == 0) {
                ui_show_message("Success", "WAV exported to output.wav");
            } else {
                ui_show_message("Error", "WAV export failed. Is eSpeak NG installed?");
            }
        }
    }
}

/* -------------------------------------------------------------------------
 * handle_about - display project information screen
 * ---------------------------------------------------------------------- */
static void handle_about(void)
{
    const char *lines[] = {
        "OpenVoice AAC Communicator v1.0",
        "",
        "A free and open-source Augmentative and",
        "Alternative Communication (AAC) system",
        "for users with speech impairments.",
        "",
        "Built with: C99 · ncurses · eSpeak NG",
        "",
        "Press any key to return...",
        NULL
    };
    ui_show_info_screen("About OpenVoice", lines);
}
