/*
 * OpenVoice AAC Communicator
 * src/phrase.c - Quick-Phrase Board Implementation
 *
 * Loads phrases from a plain-text file and provides a hard-coded fallback
 * set of emergency / high-frequency AAC phrases.
 *
 * Standard: C99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/phrase.h"

/* -------------------------------------------------------------------------
 * Internal helper: trim leading/trailing whitespace in-place
 * ---------------------------------------------------------------------- */
static void trim(char *s)
{
    if (!s) return;

    /* Trim trailing whitespace */
    int len = (int)strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }

    /* Trim leading whitespace */
    int start = 0;
    while (s[start] && isspace((unsigned char)s[start])) start++;
    if (start > 0) {
        memmove(s, s + start, (size_t)(len - start + 1));
    }
}

/* -------------------------------------------------------------------------
 * phrase_list_init
 * ---------------------------------------------------------------------- */
void phrase_list_init(PhraseList *list)
{
    if (!list) return;
    memset(list, 0, sizeof(*list));
    list->count = 0;
}

/* -------------------------------------------------------------------------
 * phrase_add
 * ---------------------------------------------------------------------- */
int phrase_add(PhraseList *list, const char *text)
{
    if (!list || !text) return -1;
    if (list->count >= MAX_PHRASES) return -1;

    strncpy(list->items[list->count].text, text, MAX_PHRASE_LEN - 1);
    list->items[list->count].text[MAX_PHRASE_LEN - 1] = '\0';
    list->count++;
    return 0;
}

/* -------------------------------------------------------------------------
 * phrase_load
 * ---------------------------------------------------------------------- */
int phrase_load(PhraseList *list, const char *path)
{
    if (!list || !path) return -1;

    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    char line[MAX_PHRASE_LEN + 2];
    while (fgets(line, sizeof(line), fp)) {
        trim(line);

        /* Skip comment lines and blank lines */
        if (line[0] == '#' || line[0] == '\0') continue;

        if (phrase_add(list, line) != 0) {
            /* List full; stop reading */
            break;
        }
    }

    fclose(fp);
    return 0;
}

/* -------------------------------------------------------------------------
 * phrase_load_defaults
 *
 * Standard AAC quick-communication phrases recommended by AAC practitioners.
 * ---------------------------------------------------------------------- */
void phrase_load_defaults(PhraseList *list)
{
    if (!list) return;

    static const char *defaults[] = {
        "Yes",
        "No",
        "Help",
        "Call Family",
        "I Need Water",
        "I Need Medicine",
        "Thank You",
        "Please Wait",
        "I Am In Pain",
        "I Need A Doctor",
        "I Need To Rest",
        "Please Repeat That",
        "I Do Not Understand",
        "Can You Help Me?",
        "I Am Hungry",
        "I Am Cold",
        "I Am Hot",
        "Please Call 911",
        "I Love You",
        "Good Morning",
        NULL
    };

    for (int i = 0; defaults[i] != NULL; i++) {
        if (phrase_add(list, defaults[i]) != 0) break;
    }
}

/* -------------------------------------------------------------------------
 * phrase_list_free
 * ---------------------------------------------------------------------- */
void phrase_list_free(PhraseList *list)
{
    /* All storage is stack-allocated inside PhraseList; nothing to free.
     * Zero the struct to invalidate stale pointers as a safety measure. */
    if (list) {
        memset(list, 0, sizeof(*list));
    }
}
