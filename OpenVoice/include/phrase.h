/*
 * OpenVoice AAC Communicator
 * include/phrase.h - Quick-Phrase Board API
 *
 * Defines the Phrase and PhraseList types and declares all phrase-
 * management functions.
 *
 * Standard: C99
 */

#ifndef OPENVOICE_PHRASE_H
#define OPENVOICE_PHRASE_H

/* -------------------------------------------------------------------------
 * Limits
 * ---------------------------------------------------------------------- */
#define MAX_PHRASE_LEN   128   /* maximum characters in one phrase         */
#define MAX_PHRASES      64    /* maximum phrases held in a PhraseList      */
#define PHRASES_PATH     "data/phrases.txt"

/* -------------------------------------------------------------------------
 * Data types
 * ---------------------------------------------------------------------- */

/** A single AAC quick phrase. */
typedef struct {
    char text[MAX_PHRASE_LEN];   /**< Phrase text shown on the board. */
} Phrase;

/** A resizable (up to MAX_PHRASES) collection of Phrase entries. */
typedef struct {
    Phrase items[MAX_PHRASES];   /**< Array of phrases.               */
    int    count;                /**< Number of valid entries.         */
} PhraseList;

/* -------------------------------------------------------------------------
 * Functions
 * ---------------------------------------------------------------------- */

/**
 * phrase_list_init - zero-initialise a PhraseList.
 *
 * @param list  PhraseList to initialise.
 */
void phrase_list_init(PhraseList *list);

/**
 * phrase_load - read phrases from a plain-text file (one phrase per line).
 *
 * Lines starting with '#' and empty lines are ignored.
 *
 * @param list  Destination PhraseList.
 * @param path  Path to the phrases file.
 * Returns 0 on success, -1 if the file cannot be opened.
 */
int phrase_load(PhraseList *list, const char *path);

/**
 * phrase_load_defaults - populate list with built-in emergency phrases.
 *
 * Used as a fallback when phrases.txt is unavailable.
 *
 * @param list  Destination PhraseList.
 */
void phrase_load_defaults(PhraseList *list);

/**
 * phrase_add - append a single phrase to list.
 *
 * @param list  Target PhraseList.
 * @param text  Phrase text (will be truncated to MAX_PHRASE_LEN-1).
 * Returns 0 on success, -1 if the list is full.
 */
int phrase_add(PhraseList *list, const char *text);

/**
 * phrase_list_free - release any resources held by list.
 *
 * Currently a no-op because storage is stack-allocated, but provided for
 * forward-compatibility if dynamic allocation is added later.
 *
 * @param list  PhraseList to free.
 */
void phrase_list_free(PhraseList *list);

#endif /* OPENVOICE_PHRASE_H */
