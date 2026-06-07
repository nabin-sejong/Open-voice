/*
 * OpenVoice AAC Communicator
 * tests/test_speech.c - Unit Tests: Speech Module
 *
 * Tests speech_escape_text and speech_build_command without actually
 * invoking eSpeak NG (no audio device required).
 *
 * Compile:
 *   gcc -std=c99 -Wall -Wextra -I../include \
 *       test_speech.c ../src/speech.c ../src/profile.c -o test_speech
 *
 * Run:
 *   ./test_speech
 *
 * Standard: C99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/speech.h"
#include "../include/profile.h"

/* -------------------------------------------------------------------------
 * Minimal test harness
 * ---------------------------------------------------------------------- */
static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)     static void name(void)
#define RUN(name)      do { printf("  %-44s", #name); \
                            tests_run++; name();        \
                            printf("PASS\n"); tests_passed++; } while(0)
#define ASSERT(cond)   do { if (!(cond)) {              \
    printf("FAIL\n    Assertion failed: %s (%s:%d)\n",  \
           #cond, __FILE__, __LINE__);                   \
    tests_failed++; return; } } while(0)

/* -------------------------------------------------------------------------
 * Helper: make a default profile for testing
 * ---------------------------------------------------------------------- */
static Profile make_default_profile(void)
{
    Profile p;
    profile_init(&p);
    profile_set_defaults(&p);
    return p;
}

/* -------------------------------------------------------------------------
 * speech_escape_text tests
 * ---------------------------------------------------------------------- */

TEST(test_escape_no_special_chars)
{
    char buf[64];
    ASSERT(speech_escape_text(buf, sizeof(buf), "Hello World") == 0);
    ASSERT(strcmp(buf, "Hello World") == 0);
}

TEST(test_escape_single_quote)
{
    char buf[64];
    /* Input:   it's
     * Output:  it'\''s  (single-quote escaping for sh)
     */
    ASSERT(speech_escape_text(buf, sizeof(buf), "it's") == 0);
    ASSERT(strcmp(buf, "it'\\''s") == 0);
}

TEST(test_escape_multiple_quotes)
{
    char buf[128];
    ASSERT(speech_escape_text(buf, sizeof(buf), "I'm can't stop") == 0);
    /* Each ' becomes '\'' */
    ASSERT(strstr(buf, "'\\''") != NULL);
}

TEST(test_escape_empty_string)
{
    char buf[16];
    ASSERT(speech_escape_text(buf, sizeof(buf), "") == 0);
    ASSERT(buf[0] == '\0');
}

TEST(test_escape_buffer_too_small)
{
    char buf[4]; /* too small for "it's" escape */
    int rc = speech_escape_text(buf, sizeof(buf), "it's");
    ASSERT(rc == -1);
}

/* -------------------------------------------------------------------------
 * speech_build_command tests
 * ---------------------------------------------------------------------- */

TEST(test_build_command_basic)
{
    Profile p = make_default_profile();
    char cmd[512];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Hello", &p, NULL) == 0);

    /* Must contain the binary name */
    ASSERT(strstr(cmd, "espeak-ng") != NULL);

    /* Must include pitch / speed / volume flags */
    ASSERT(strstr(cmd, "-p ") != NULL);
    ASSERT(strstr(cmd, "-s ") != NULL);
    ASSERT(strstr(cmd, "-a ") != NULL);

    /* Must include the text */
    ASSERT(strstr(cmd, "Hello") != NULL);
}

TEST(test_build_command_with_wav_output)
{
    Profile p = make_default_profile();
    char cmd[512];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, "out.wav") == 0);
    ASSERT(strstr(cmd, "-w") != NULL);
    ASSERT(strstr(cmd, "out.wav") != NULL);
}

TEST(test_build_command_language_flag)
{
    Profile p = make_default_profile();
    strncpy(p.language, "fr", LANG_TAG_LEN - 1);
    char cmd[512];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Bonjour", &p, NULL) == 0);
    ASSERT(strstr(cmd, "fr") != NULL);
}

TEST(test_build_command_buffer_too_small)
{
    Profile p = make_default_profile();
    char cmd[8]; /* way too small */
    int rc = speech_build_command(cmd, sizeof(cmd), "Hello", &p, NULL);
    ASSERT(rc == -1);
}

TEST(test_build_command_null_text)
{
    Profile p = make_default_profile();
    char cmd[512];
    ASSERT(speech_build_command(cmd, sizeof(cmd), NULL, &p, NULL) == -1);
}

TEST(test_build_command_null_profile)
{
    char cmd[512];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Hello", NULL, NULL) == -1);
}

TEST(test_build_command_pitch_in_range)
{
    Profile p = make_default_profile();
    p.pitch = 75;
    char cmd[512];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, NULL) == 0);
    ASSERT(strstr(cmd, "75") != NULL);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */
int main(void)
{
    printf("\n=== Speech Module Tests ===\n\n");

    printf("  -- speech_escape_text --\n");
    RUN(test_escape_no_special_chars);
    RUN(test_escape_single_quote);
    RUN(test_escape_multiple_quotes);
    RUN(test_escape_empty_string);
    RUN(test_escape_buffer_too_small);

    printf("\n  -- speech_build_command --\n");
    RUN(test_build_command_basic);
    RUN(test_build_command_with_wav_output);
    RUN(test_build_command_language_flag);
    RUN(test_build_command_buffer_too_small);
    RUN(test_build_command_null_text);
    RUN(test_build_command_null_profile);
    RUN(test_build_command_pitch_in_range);

    printf("\n--- Results: %d/%d passed", tests_passed, tests_run);
    if (tests_failed > 0) {
        printf("  (%d FAILED)", tests_failed);
    }
    printf(" ---\n\n");

    return (tests_failed > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
