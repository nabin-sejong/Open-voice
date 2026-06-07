/*
 * OpenVoice AAC Communicator
 * tests/test_phrase.c - Unit Tests: Phrase Module
 *
 * Tests phrase_list_init, phrase_add, phrase_load, phrase_load_defaults,
 * and phrase_list_free.
 *
 * Compile:
 *   gcc -std=c99 -Wall -Wextra -I../include \
 *       test_phrase.c ../src/phrase.c -o test_phrase
 *
 * Run:
 *   ./test_phrase
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

#define TEST(name)     static void name(void)
#define RUN(name)      do { printf("  %-40s", #name); \
                            tests_run++; name();        \
                            printf("PASS\n"); tests_passed++; } while(0)
#define ASSERT(cond)   do { if (!(cond)) {              \
    printf("FAIL\n    Assertion failed: %s (%s:%d)\n",  \
           #cond, __FILE__, __LINE__);                   \
    tests_failed++; return; } } while(0)

/* -------------------------------------------------------------------------
 * Tests
 * ---------------------------------------------------------------------- */

TEST(test_phrase_list_init)
{
    PhraseList list;
    phrase_list_init(&list);
    ASSERT(list.count == 0);
}

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

TEST(test_phrase_add_truncation)
{
    /* Phrase longer than MAX_PHRASE_LEN-1 should be silently truncated */
    PhraseList list;
    phrase_list_init(&list);

    char long_phrase[MAX_PHRASE_LEN + 32];
    memset(long_phrase, 'A', sizeof(long_phrase) - 1);
    long_phrase[sizeof(long_phrase) - 1] = '\0';

    ASSERT(phrase_add(&list, long_phrase) == 0);
    ASSERT((int)strlen(list.items[0].text) == MAX_PHRASE_LEN - 1);
}

TEST(test_phrase_add_list_full)
{
    PhraseList list;
    phrase_list_init(&list);
    for (int i = 0; i < MAX_PHRASES; i++) {
        ASSERT(phrase_add(&list, "phrase") == 0);
    }
    /* One more should fail */
    ASSERT(phrase_add(&list, "overflow") == -1);
    ASSERT(list.count == MAX_PHRASES);
}

TEST(test_phrase_load_defaults)
{
    PhraseList list;
    phrase_list_init(&list);
    phrase_load_defaults(&list);
    ASSERT(list.count > 0);

    /* "Yes" must be the first default phrase */
    ASSERT(strcmp(list.items[0].text, "Yes") == 0);
}

TEST(test_phrase_load_from_file)
{
    const char *tmp = "/tmp/openvoice_test_phrases.txt";

    FILE *fp = fopen(tmp, "w");
    ASSERT(fp != NULL);
    fprintf(fp, "# comment line\n");
    fprintf(fp, "\n");
    fprintf(fp, "Good morning\n");
    fprintf(fp, "I need help\n");
    fprintf(fp, "Thank you\n");
    fclose(fp);

    PhraseList list;
    phrase_list_init(&list);
    ASSERT(phrase_load(&list, tmp) == 0);
    ASSERT(list.count == 3);
    ASSERT(strcmp(list.items[0].text, "Good morning") == 0);
    ASSERT(strcmp(list.items[1].text, "I need help")  == 0);
    ASSERT(strcmp(list.items[2].text, "Thank you")    == 0);

    remove(tmp);
}

TEST(test_phrase_load_missing_file)
{
    PhraseList list;
    phrase_list_init(&list);
    int rc = phrase_load(&list, "/tmp/no_such_phrases_999.txt");
    ASSERT(rc == -1);
    ASSERT(list.count == 0);
}

TEST(test_phrase_list_free)
{
    PhraseList list;
    phrase_list_init(&list);
    phrase_load_defaults(&list);
    ASSERT(list.count > 0);
    phrase_list_free(&list);
    ASSERT(list.count == 0);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */
int main(void)
{
    printf("\n=== Phrase Module Tests ===\n\n");

    RUN(test_phrase_list_init);
    RUN(test_phrase_add_single);
    RUN(test_phrase_add_multiple);
    RUN(test_phrase_add_truncation);
    RUN(test_phrase_add_list_full);
    RUN(test_phrase_load_defaults);
    RUN(test_phrase_load_from_file);
    RUN(test_phrase_load_missing_file);
    RUN(test_phrase_list_free);

    printf("\n--- Results: %d/%d passed", tests_passed, tests_run);
    if (tests_failed > 0) {
        printf("  (%d FAILED)", tests_failed);
    }
    printf(" ---\n\n");

    return (tests_failed > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
