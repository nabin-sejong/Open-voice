/*
 * OpenVoice AAC Communicator
 * tests/test_phrase.c - Unit Tests: Phrase Module
 *
 * Covers:
 *   - At least 8 phrases present in the default list
 *   - Valid input: normal phrases, special characters, all 12 demo phrases
 *   - Invalid input: NULL args, list overflow, overly-long phrase truncation
 *   - Phrase file handling: load, missing file, comments/blanks, 8+ phrase file,
 *                           null path, overfull file
 *
 * Compile (from repo root):
 *   gcc -std=c99 -Wall -Wextra -I include \
 *       tests/test_phrase.c src/phrase.c -o tests/test_phrase.exe
 *
 * Standard: C99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/phrase.h"

/* -------------------------------------------------------------------------
 * Minimal test harness
 * ---------------------------------------------------------------------- */
static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)   static void name(void)
#define RUN(name)    do { printf("  %-52s", #name);                  \
                         tests_run++;                                  \
                         int _f_before = tests_failed; name();         \
                         if (tests_failed == _f_before) {              \
                             printf("PASS\n"); tests_passed++;         \
                         } } while(0)
#define ASSERT(cond) do { if (!(cond)) {                \
    printf("FAIL\n    Assertion failed: %s (%s:%d)\n",  \
           #cond, __FILE__, __LINE__);                   \
    tests_failed++; return; } } while(0)

/* Helper: write a temp phrase file and return the path.
 * Caller is responsible for remove(path) afterward.            */
static void write_tmp_phrases(const char *path,
                              const char * const *lines, int n)
{
    FILE *fp = fopen(path, "w");
    if (!fp) { fprintf(stderr, "Cannot open %s\n", path); return; }
    for (int i = 0; i < n; i++) fprintf(fp, "%s\n", lines[i]);
    fclose(fp);
}

/* =========================================================================
 * Section 1 — phrase_list_init
 * ====================================================================== */

TEST(test_phrase_list_init_zeros_count)
{
    PhraseList list;
    /* fill with garbage to catch any false-zero behaviour */
    memset(&list, 0xFF, sizeof(list));
    phrase_list_init(&list);
    ASSERT(list.count == 0);
}

TEST(test_phrase_list_init_null_safe)
{
    /* Must not crash on NULL — no return value to check */
    phrase_list_init(NULL);
    ASSERT(1); /* reached here without crash */
}

/* =========================================================================
 * Section 2 — phrase_add: valid input
 * ====================================================================== */

TEST(test_phrase_add_single)
{
    PhraseList list;
    phrase_list_init(&list);
    ASSERT(phrase_add(&list, "Hello") == 0);
    ASSERT(list.count == 1);
    ASSERT(strcmp(list.items[0].text, "Hello") == 0);
}

TEST(test_phrase_add_multiple)
{
    PhraseList list;
    phrase_list_init(&list);
    phrase_add(&list, "Yes");
    phrase_add(&list, "No");
    phrase_add(&list, "Help");
    ASSERT(list.count == 3);
    ASSERT(strcmp(list.items[0].text, "Yes")  == 0);
    ASSERT(strcmp(list.items[1].text, "No")   == 0);
    ASSERT(strcmp(list.items[2].text, "Help") == 0);
}

TEST(test_phrase_add_all_12_demo_phrases)
{
    /* Verify that all 12 curated demo phrases can be stored verbatim */
    static const char * const demo[] = {
        "Yes", "No", "Help", "Thank You",
        "I Need Water", "I Need Medicine", "I Am In Pain", "Call 911",
        "Please Wait", "I Don't Understand", "Hello", "I Love You"
    };
    PhraseList list;
    phrase_list_init(&list);
    for (int i = 0; i < 12; i++) {
        ASSERT(phrase_add(&list, demo[i]) == 0);
    }
    ASSERT(list.count == 12);
    for (int i = 0; i < 12; i++) {
        ASSERT(strcmp(list.items[i].text, demo[i]) == 0);
    }
}

TEST(test_phrase_add_apostrophe_preserved)
{
    /* Single-quote in phrase text must be stored as-is */
    PhraseList list;
    phrase_list_init(&list);
    ASSERT(phrase_add(&list, "I Don't Understand") == 0);
    ASSERT(strcmp(list.items[0].text, "I Don't Understand") == 0);
}

TEST(test_phrase_add_numbers_and_punctuation)
{
    PhraseList list;
    phrase_list_init(&list);
    ASSERT(phrase_add(&list, "Call 911") == 0);
    ASSERT(strcmp(list.items[0].text, "Call 911") == 0);
}

TEST(test_phrase_add_returns_zero_on_success)
{
    PhraseList list;
    phrase_list_init(&list);
    int rc = phrase_add(&list, "Test");
    ASSERT(rc == 0);
}

/* =========================================================================
 * Section 3 — phrase_add: invalid / edge input
 * ====================================================================== */

TEST(test_phrase_add_null_list)
{
    ASSERT(phrase_add(NULL, "Hello") == -1);
}

TEST(test_phrase_add_null_text)
{
    PhraseList list;
    phrase_list_init(&list);
    ASSERT(phrase_add(&list, NULL) == -1);
    ASSERT(list.count == 0); /* list must not be modified */
}

TEST(test_phrase_add_list_full)
{
    PhraseList list;
    phrase_list_init(&list);
    for (int i = 0; i < MAX_PHRASES; i++) {
        ASSERT(phrase_add(&list, "phrase") == 0);
    }
    /* One beyond capacity must fail */
    ASSERT(phrase_add(&list, "overflow") == -1);
    ASSERT(list.count == MAX_PHRASES);
}

TEST(test_phrase_add_long_phrase_truncated)
{
    /* A phrase longer than MAX_PHRASE_LEN-1 must be silently truncated */
    PhraseList list;
    phrase_list_init(&list);

    char long_phrase[MAX_PHRASE_LEN + 64];
    memset(long_phrase, 'X', sizeof(long_phrase) - 1);
    long_phrase[sizeof(long_phrase) - 1] = '\0';

    ASSERT(phrase_add(&list, long_phrase) == 0);
    ASSERT(list.count == 1);
    ASSERT((int)strlen(list.items[0].text) == MAX_PHRASE_LEN - 1);
}

TEST(test_phrase_add_exact_max_len_phrase)
{
    /* Phrase exactly MAX_PHRASE_LEN-1 characters must fit without truncation */
    PhraseList list;
    phrase_list_init(&list);

    char exact[MAX_PHRASE_LEN];
    memset(exact, 'B', MAX_PHRASE_LEN - 1);
    exact[MAX_PHRASE_LEN - 1] = '\0';

    ASSERT(phrase_add(&list, exact) == 0);
    ASSERT((int)strlen(list.items[0].text) == MAX_PHRASE_LEN - 1);
}

/* =========================================================================
 * Section 4 — phrase_load_defaults: at least 8 phrases present
 * ====================================================================== */

TEST(test_phrase_load_defaults_at_least_8)
{
    PhraseList list;
    phrase_list_init(&list);
    phrase_load_defaults(&list);
    /* AAC minimum viable board requires at least 8 core phrases */
    ASSERT(list.count >= 8);
}

TEST(test_phrase_load_defaults_first_is_yes)
{
    PhraseList list;
    phrase_list_init(&list);
    phrase_load_defaults(&list);
    ASSERT(strcmp(list.items[0].text, "Yes") == 0);
}

TEST(test_phrase_load_defaults_contains_no)
{
    PhraseList list;
    phrase_list_init(&list);
    phrase_load_defaults(&list);
    int found = 0;
    for (int i = 0; i < list.count; i++) {
        if (strcmp(list.items[i].text, "No") == 0) { found = 1; break; }
    }
    ASSERT(found);
}

TEST(test_phrase_load_defaults_contains_help)
{
    PhraseList list;
    phrase_list_init(&list);
    phrase_load_defaults(&list);
    int found = 0;
    for (int i = 0; i < list.count; i++) {
        if (strcmp(list.items[i].text, "Help") == 0) { found = 1; break; }
    }
    ASSERT(found);
}

TEST(test_phrase_load_defaults_contains_emergency_phrases)
{
    /* At least one emergency/medical phrase must be in the built-in set */
    static const char * const emergency[] = {
        "I Am In Pain", "I Need Medicine", "Call 911",
        "I Need A Doctor", "Please Call 911", NULL
    };
    PhraseList list;
    phrase_list_init(&list);
    phrase_load_defaults(&list);

    int found = 0;
    for (int i = 0; i < list.count && !found; i++) {
        for (int j = 0; emergency[j] != NULL; j++) {
            if (strcmp(list.items[i].text, emergency[j]) == 0) {
                found = 1; break;
            }
        }
    }
    ASSERT(found);
}

/* =========================================================================
 * Section 5 — phrase_load (file): valid file handling
 * ====================================================================== */

TEST(test_phrase_load_basic_3_phrases)
{
    const char *tmp = "openvoice_test_phrases_basic.txt";
    static const char * const lines[] = {
        "# comment", "", "Good morning", "I need help", "Thank you"
    };
    write_tmp_phrases(tmp, lines, 5);

    PhraseList list;
    phrase_list_init(&list);
    ASSERT(phrase_load(&list, tmp) == 0);
    ASSERT(list.count == 3);
    ASSERT(strcmp(list.items[0].text, "Good morning") == 0);
    ASSERT(strcmp(list.items[1].text, "I need help")  == 0);
    ASSERT(strcmp(list.items[2].text, "Thank you")    == 0);

    remove(tmp);
}

TEST(test_phrase_load_8_or_more_phrases)
{
    /* File with 8 real phrases must load all 8 */
    const char *tmp = "openvoice_test_phrases_8.txt";
    static const char * const lines[] = {
        "Yes", "No", "Help", "Thank You",
        "I Need Water", "I Need Medicine", "I Am In Pain", "Call 911"
    };
    write_tmp_phrases(tmp, lines, 8);

    PhraseList list;
    phrase_list_init(&list);
    ASSERT(phrase_load(&list, tmp) == 0);
    ASSERT(list.count == 8);
    ASSERT(strcmp(list.items[0].text, "Yes")      == 0);
    ASSERT(strcmp(list.items[7].text, "Call 911") == 0);

    remove(tmp);
}

TEST(test_phrase_load_12_demo_phrases_from_file)
{
    /* Simulate the actual data/phrases.txt used in the demo */
    const char *tmp = "openvoice_test_phrases_12.txt";
    static const char * const lines[] = {
        "# OpenVoice Demo Phrase Board",
        "Yes", "No", "Help", "Thank You",
        "I Need Water", "I Need Medicine", "I Am In Pain", "Call 911",
        "Please Wait", "I Don't Understand", "Hello", "I Love You"
    };
    write_tmp_phrases(tmp, lines, 13);

    PhraseList list;
    phrase_list_init(&list);
    ASSERT(phrase_load(&list, tmp) == 0);
    ASSERT(list.count == 12);
    ASSERT(strcmp(list.items[0].text,  "Yes")               == 0);
    ASSERT(strcmp(list.items[3].text,  "Thank You")         == 0);
    ASSERT(strcmp(list.items[9].text,  "I Don't Understand") == 0);
    ASSERT(strcmp(list.items[11].text, "I Love You")        == 0);

    remove(tmp);
}

TEST(test_phrase_load_skips_comment_lines)
{
    const char *tmp = "openvoice_test_phrases_comments.txt";
    static const char * const lines[] = {
        "# This is a comment", "# Another comment", "Real Phrase"
    };
    write_tmp_phrases(tmp, lines, 3);

    PhraseList list;
    phrase_list_init(&list);
    ASSERT(phrase_load(&list, tmp) == 0);
    ASSERT(list.count == 1);
    ASSERT(strcmp(list.items[0].text, "Real Phrase") == 0);

    remove(tmp);
}

TEST(test_phrase_load_skips_blank_lines)
{
    const char *tmp = "openvoice_test_phrases_blanks.txt";
    static const char * const lines[] = {
        "", "  ", "Alpha", "", "Beta", ""
    };
    write_tmp_phrases(tmp, lines, 6);

    PhraseList list;
    phrase_list_init(&list);
    ASSERT(phrase_load(&list, tmp) == 0);
    ASSERT(list.count == 2);
    ASSERT(strcmp(list.items[0].text, "Alpha") == 0);
    ASSERT(strcmp(list.items[1].text, "Beta")  == 0);

    remove(tmp);
}

TEST(test_phrase_load_all_comments_gives_empty_list)
{
    const char *tmp = "openvoice_test_phrases_allcomments.txt";
    static const char * const lines[] = {
        "# comment 1", "# comment 2", "# comment 3"
    };
    write_tmp_phrases(tmp, lines, 3);

    PhraseList list;
    phrase_list_init(&list);
    ASSERT(phrase_load(&list, tmp) == 0);
    ASSERT(list.count == 0);

    remove(tmp);
}

TEST(test_phrase_load_stops_when_list_full)
{
    /* Write MAX_PHRASES + 5 phrases; list must cap at MAX_PHRASES */
    const char *tmp = "openvoice_test_phrases_overfull.txt";
    FILE *fp = fopen(tmp, "w");
    ASSERT(fp != NULL);
    for (int i = 0; i < MAX_PHRASES + 5; i++) {
        fprintf(fp, "Phrase %d\n", i);
    }
    fclose(fp);

    PhraseList list;
    phrase_list_init(&list);
    ASSERT(phrase_load(&list, tmp) == 0);
    ASSERT(list.count == MAX_PHRASES);

    remove(tmp);
}

/* =========================================================================
 * Section 6 — phrase_load (file): invalid / edge file handling
 * ====================================================================== */

TEST(test_phrase_load_missing_file)
{
    PhraseList list;
    phrase_list_init(&list);
    int rc = phrase_load(&list, "no_such_phrases_openvoice_99999.txt");
    ASSERT(rc == -1);
    ASSERT(list.count == 0);
}

TEST(test_phrase_load_null_path)
{
    PhraseList list;
    phrase_list_init(&list);
    ASSERT(phrase_load(&list, NULL) == -1);
    ASSERT(list.count == 0);
}

TEST(test_phrase_load_null_list)
{
    /* Must not crash and must return error */
    ASSERT(phrase_load(NULL, "phrases.txt") == -1);
}

TEST(test_phrase_load_both_null)
{
    ASSERT(phrase_load(NULL, NULL) == -1);
}

/* =========================================================================
 * Section 7 — phrase_list_free
 * ====================================================================== */

TEST(test_phrase_list_free_resets_count)
{
    PhraseList list;
    phrase_list_init(&list);
    phrase_load_defaults(&list);
    ASSERT(list.count > 0);
    phrase_list_free(&list);
    ASSERT(list.count == 0);
}

TEST(test_phrase_list_free_null_safe)
{
    phrase_list_free(NULL); /* must not crash */
    ASSERT(1);
}

/* =========================================================================
 * main
 * ====================================================================== */
int main(void)
{
    printf("\n=== Phrase Module Tests ===\n\n");

    printf("  -- phrase_list_init --\n");
    RUN(test_phrase_list_init_zeros_count);
    RUN(test_phrase_list_init_null_safe);

    printf("\n  -- phrase_add: valid input --\n");
    RUN(test_phrase_add_single);
    RUN(test_phrase_add_multiple);
    RUN(test_phrase_add_all_12_demo_phrases);
    RUN(test_phrase_add_apostrophe_preserved);
    RUN(test_phrase_add_numbers_and_punctuation);
    RUN(test_phrase_add_returns_zero_on_success);

    printf("\n  -- phrase_add: invalid / edge input --\n");
    RUN(test_phrase_add_null_list);
    RUN(test_phrase_add_null_text);
    RUN(test_phrase_add_list_full);
    RUN(test_phrase_add_long_phrase_truncated);
    RUN(test_phrase_add_exact_max_len_phrase);

    printf("\n  -- phrase_load_defaults: >= 8 phrases --\n");
    RUN(test_phrase_load_defaults_at_least_8);
    RUN(test_phrase_load_defaults_first_is_yes);
    RUN(test_phrase_load_defaults_contains_no);
    RUN(test_phrase_load_defaults_contains_help);
    RUN(test_phrase_load_defaults_contains_emergency_phrases);

    printf("\n  -- phrase_load (file): valid handling --\n");
    RUN(test_phrase_load_basic_3_phrases);
    RUN(test_phrase_load_8_or_more_phrases);
    RUN(test_phrase_load_12_demo_phrases_from_file);
    RUN(test_phrase_load_skips_comment_lines);
    RUN(test_phrase_load_skips_blank_lines);
    RUN(test_phrase_load_all_comments_gives_empty_list);
    RUN(test_phrase_load_stops_when_list_full);

    printf("\n  -- phrase_load (file): invalid / edge handling --\n");
    RUN(test_phrase_load_missing_file);
    RUN(test_phrase_load_null_path);
    RUN(test_phrase_load_null_list);
    RUN(test_phrase_load_both_null);

    printf("\n  -- phrase_list_free --\n");
    RUN(test_phrase_list_free_resets_count);
    RUN(test_phrase_list_free_null_safe);

    printf("\n--- Results: %d/%d passed", tests_passed, tests_run);
    if (tests_failed > 0) {
        printf("  (%d FAILED)", tests_failed);
    }
    printf(" ---\n\n");

    return (tests_failed > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
