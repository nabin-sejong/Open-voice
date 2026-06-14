/*
 * OpenVoice AAC Communicator
 * tests/test_profile.c - Unit Tests: Profile Module
 *
 * Tests profile_set_defaults, profile_clamp, profile_save, and profile_load.
 * Uses a simple hand-rolled test framework (no external dependencies).
 *
 * Compile:
 *   gcc -std=c99 -Wall -Wextra -I../include \
 *       test_profile.c ../src/profile.c -o test_profile
 *
 * Run:
 *   ./test_profile
 *
 * Standard: C99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/profile.h"

/* -------------------------------------------------------------------------
 * Minimal test harness
 * ---------------------------------------------------------------------- */
static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)     static void name(void)
#define RUN(name)      do { printf("  %-40s", #name);                  \
                            tests_run++;                                 \
                            int _f_before = tests_failed; name();        \
                            if (tests_failed == _f_before) {             \
                                printf("PASS\n"); tests_passed++;        \
                            } } while(0)
#define ASSERT(cond)   do { if (!(cond)) {              \
    printf("FAIL\n    Assertion failed: %s (%s:%d)\n",  \
           #cond, __FILE__, __LINE__);                   \
    tests_failed++; return; } } while(0)

/* -------------------------------------------------------------------------
 * Tests
 * ---------------------------------------------------------------------- */

TEST(test_profile_init_zeros)
{
    Profile p;
    memset(&p, 0xFF, sizeof(p)); /* fill with garbage */
    profile_init(&p);
    ASSERT(p.pitch  == 0);
    ASSERT(p.speed  == 0);
    ASSERT(p.volume == 0);
    ASSERT(p.language[0] == '\0');
}

TEST(test_profile_set_defaults)
{
    Profile p;
    profile_init(&p);
    profile_set_defaults(&p);
    ASSERT(p.pitch  == PITCH_DEF);
    ASSERT(p.speed  == SPEED_DEF);
    ASSERT(p.volume == VOLUME_DEF);
    ASSERT(strcmp(p.language, "en-us") == 0);
}

TEST(test_profile_clamp_low)
{
    Profile p;
    profile_init(&p);
    p.pitch  = PITCH_MIN  - 10;
    p.speed  = SPEED_MIN  - 10;
    p.volume = VOLUME_MIN - 10;
    strncpy(p.language, "en", LANG_TAG_LEN - 1);
    profile_clamp(&p);
    ASSERT(p.pitch  == PITCH_MIN);
    ASSERT(p.speed  == SPEED_MIN);
    ASSERT(p.volume == VOLUME_MIN);
}

TEST(test_profile_clamp_high)
{
    Profile p;
    profile_init(&p);
    p.pitch  = PITCH_MAX  + 10;
    p.speed  = SPEED_MAX  + 10;
    p.volume = VOLUME_MAX + 10;
    strncpy(p.language, "en", LANG_TAG_LEN - 1);
    profile_clamp(&p);
    ASSERT(p.pitch  == PITCH_MAX);
    ASSERT(p.speed  == SPEED_MAX);
    ASSERT(p.volume == VOLUME_MAX);
}

TEST(test_profile_clamp_restores_empty_language)
{
    Profile p;
    profile_init(&p);
    profile_set_defaults(&p);
    p.language[0] = '\0';
    profile_clamp(&p);
    ASSERT(p.language[0] != '\0');
}

TEST(test_profile_save_and_load)
{
    const char *tmp = "openvoice_test_profile.cfg";

    Profile orig;
    profile_init(&orig);
    orig.pitch  = 7;    /* in new range -10..+10 */
    orig.speed  = 200;
    orig.volume = 80;
    strncpy(orig.language, "en-gb", LANG_TAG_LEN - 1);

    ASSERT(profile_save(&orig, tmp) == 0);

    Profile loaded;
    profile_init(&loaded);
    ASSERT(profile_load(&loaded, tmp) == 0);

    ASSERT(loaded.pitch  == 7);
    ASSERT(loaded.speed  == 200);
    ASSERT(loaded.volume == 80);
    ASSERT(strcmp(loaded.language, "en-gb") == 0);

    remove(tmp);
}

TEST(test_profile_load_missing_file)
{
    Profile p;
    profile_init(&p);
    int rc = profile_load(&p, "no_such_file_openvoice_12345.cfg");
    ASSERT(rc == -1);
}

TEST(test_profile_load_partial)
{
    /* Only pitch is present in the file; others should stay at defaults */
    const char *tmp = "openvoice_partial_profile.cfg";

    FILE *fp = fopen(tmp, "w");
    ASSERT(fp != NULL);
    fprintf(fp, "# comment\n");
    fprintf(fp, "pitch=5\n");
    fclose(fp);

    Profile p;
    profile_init(&p);
    profile_set_defaults(&p);
    ASSERT(profile_load(&p, tmp) == 0);
    ASSERT(p.pitch  == 5);
    ASSERT(p.speed  == SPEED_DEF);
    ASSERT(p.volume == VOLUME_DEF);

    remove(tmp);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */
int main(void)
{
    printf("\n=== Profile Module Tests ===\n\n");

    RUN(test_profile_init_zeros);
    RUN(test_profile_set_defaults);
    RUN(test_profile_clamp_low);
    RUN(test_profile_clamp_high);
    RUN(test_profile_clamp_restores_empty_language);
    RUN(test_profile_save_and_load);
    RUN(test_profile_load_missing_file);
    RUN(test_profile_load_partial);

    printf("\n--- Results: %d/%d passed", tests_passed, tests_run);
    if (tests_failed > 0) {
        printf("  (%d FAILED)", tests_failed);
    }
    printf(" ---\n\n");

    return (tests_failed > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
