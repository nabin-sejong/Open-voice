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
#define RUN(name)      do { printf("  %-44s", #name);                  \
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
     * Output:  it''s  (PowerShell single-quote doubling rule)
     */
    ASSERT(speech_escape_text(buf, sizeof(buf), "it's") == 0);
    ASSERT(strcmp(buf, "it''s") == 0);
}

TEST(test_escape_multiple_quotes)
{
    char buf[128];
    ASSERT(speech_escape_text(buf, sizeof(buf), "I'm can't stop") == 0);
    /* Each ' becomes '' */
    ASSERT(strstr(buf, "''") != NULL);
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
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Hello", &p, NULL) == 0);

    /* Must use PowerShell SAPI with SSML */
    ASSERT(strstr(cmd, "powershell")        != NULL);
    ASSERT(strstr(cmd, "SpeakSsml")         != NULL);
    ASSERT(strstr(cmd, "prosody")           != NULL);
    ASSERT(strstr(cmd, "Volume=")           != NULL);
    ASSERT(strstr(cmd, "xml:lang")          != NULL);

    /* Live-speech path: must use temp-WAV + SoundPlayer, not direct audio */
    ASSERT(strstr(cmd, "GetRandomFileName") != NULL);  /* leak-free temp path */
    ASSERT(strstr(cmd, "SoundPlayer")       != NULL);
    ASSERT(strstr(cmd, "PlaySync")          != NULL);
    ASSERT(strstr(cmd, "Remove-Item")       != NULL);

    /* Must include the text */
    ASSERT(strstr(cmd, "Hello") != NULL);
}

TEST(test_build_command_with_wav_output)
{
    Profile p = make_default_profile();
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, "out.wav") == 0);

    /* Export path: write to the specified WAV file */
    ASSERT(strstr(cmd, "SetOutputToWaveFile") != NULL);
    ASSERT(strstr(cmd, "out.wav")             != NULL);
    ASSERT(strstr(cmd, "xml:lang")            != NULL);

    /* Export path must NOT play back audio */
    ASSERT(strstr(cmd, "SoundPlayer")         == NULL);
    ASSERT(strstr(cmd, "GetTempFileName")     == NULL);
}

TEST(test_build_command_language_flag)
{
    /* SAPI uses the system default voice; setting a language tag must not
     * crash or produce an invalid command. */
    Profile p = make_default_profile();
    strncpy(p.language, "fr", LANG_TAG_LEN - 1);
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Bonjour", &p, NULL) == 0);
    ASSERT(strstr(cmd, "powershell") != NULL);
    ASSERT(strstr(cmd, "xml:lang")   != NULL);
    ASSERT(strstr(cmd, "fr")         != NULL);
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

TEST(test_build_command_rate_reflects_speed)
{
    /* 350 wpm = 2x of 175 wpm default → SSML rate=2.00 */
    Profile p = make_default_profile();
    p.speed = 350;
    char cmd[1024];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, NULL) == 0);
    ASSERT(strstr(cmd, "rate=") != NULL);
    ASSERT(strstr(cmd, "2.00")  != NULL);
}

TEST(test_build_command_pitch_in_ssml)
{
    /* pitch -5 semitones should appear as -5st in the SSML */
    Profile p = make_default_profile();
    p.pitch = -5;
    char cmd[1024];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, NULL) == 0);
    ASSERT(strstr(cmd, "-5st") != NULL);
}

/* -------------------------------------------------------------------------
 * speech_escape_xml tests
 * ---------------------------------------------------------------------- */

TEST(test_escape_xml_no_special)
{
    char buf[64];
    ASSERT(speech_escape_xml(buf, sizeof(buf), "Hello World") == 0);
    ASSERT(strcmp(buf, "Hello World") == 0);
}

TEST(test_escape_xml_ampersand)
{
    char buf[64];
    ASSERT(speech_escape_xml(buf, sizeof(buf), "cats & dogs") == 0);
    ASSERT(strcmp(buf, "cats &amp; dogs") == 0);
}

TEST(test_escape_xml_angle_brackets)
{
    char buf[64];
    ASSERT(speech_escape_xml(buf, sizeof(buf), "1<2>0") == 0);
    ASSERT(strcmp(buf, "1&lt;2&gt;0") == 0);
}

TEST(test_escape_xml_empty)
{
    char buf[16];
    ASSERT(speech_escape_xml(buf, sizeof(buf), "") == 0);
    ASSERT(buf[0] == '\0');
}

TEST(test_escape_xml_buffer_too_small)
{
    char buf[4]; /* too small for "&amp;" */
    ASSERT(speech_escape_xml(buf, sizeof(buf), "a&b") == -1);
}

/* -------------------------------------------------------------------------
 * speech_escape_text: valid input
 * ---------------------------------------------------------------------- */

TEST(test_escape_text_digits_unchanged)
{
    char buf[32];
    ASSERT(speech_escape_text(buf, sizeof(buf), "Call 911") == 0);
    ASSERT(strcmp(buf, "Call 911") == 0);
}

TEST(test_escape_text_all_ascii_printable)
{
    /* A sentence with common ASCII punctuation (no single-quote) must pass through */
    char buf[128];
    ASSERT(speech_escape_text(buf, sizeof(buf), "Hello, World! How are you?") == 0);
    ASSERT(strcmp(buf, "Hello, World! How are you?") == 0);
}

TEST(test_escape_text_preserves_xml_entities)
{
    /* After XML escaping, &amp; is fed into PS escaper — & and ; are safe */
    char buf[64];
    ASSERT(speech_escape_text(buf, sizeof(buf), "&amp;") == 0);
    ASSERT(strcmp(buf, "&amp;") == 0);
}

/* -------------------------------------------------------------------------
 * speech_escape_text: invalid input
 * ---------------------------------------------------------------------- */

TEST(test_escape_text_null_buf)
{
    ASSERT(speech_escape_text(NULL, 64, "hello") == -1);
}

TEST(test_escape_text_null_text)
{
    char buf[64];
    ASSERT(speech_escape_text(buf, sizeof(buf), NULL) == -1);
}

TEST(test_escape_text_zero_bufsize)
{
    char buf[64];
    ASSERT(speech_escape_text(buf, 0, "hello") == -1);
}

/* -------------------------------------------------------------------------
 * speech_escape_xml: valid input
 * ---------------------------------------------------------------------- */

TEST(test_escape_xml_combined)
{
    /* All three special chars at once */
    char buf[64];
    ASSERT(speech_escape_xml(buf, sizeof(buf), "a & b < c > d") == 0);
    ASSERT(strcmp(buf, "a &amp; b &lt; c &gt; d") == 0);
}

TEST(test_escape_xml_repeated_ampersand)
{
    char buf[64];
    ASSERT(speech_escape_xml(buf, sizeof(buf), "a&b&c") == 0);
    ASSERT(strcmp(buf, "a&amp;b&amp;c") == 0);
}

/* -------------------------------------------------------------------------
 * speech_escape_xml: invalid input
 * ---------------------------------------------------------------------- */

TEST(test_escape_xml_null_buf)
{
    ASSERT(speech_escape_xml(NULL, 64, "hello") == -1);
}

TEST(test_escape_xml_null_text)
{
    char buf[64];
    ASSERT(speech_escape_xml(buf, sizeof(buf), NULL) == -1);
}

TEST(test_escape_xml_zero_bufsize)
{
    char buf[64];
    ASSERT(speech_escape_xml(buf, 0, "hello") == -1);
}

/* -------------------------------------------------------------------------
 * speech_build_command: valid input — prosody / SSML correctness
 * ---------------------------------------------------------------------- */

TEST(test_build_command_positive_pitch)
{
    Profile p = make_default_profile();
    p.pitch = 5;
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, NULL) == 0);
    ASSERT(strstr(cmd, "+5st") != NULL);
}

TEST(test_build_command_negative_pitch)
{
    Profile p = make_default_profile();
    p.pitch = -10;
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, NULL) == 0);
    ASSERT(strstr(cmd, "-10st") != NULL);
}

TEST(test_build_command_zero_pitch)
{
    Profile p = make_default_profile();
    p.pitch = 0;
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, NULL) == 0);
    ASSERT(strstr(cmd, "+0st") != NULL);
}

TEST(test_build_command_slow_rate)
{
    /* 88 wpm ≈ 0.50 of 175 wpm baseline → rate=0.50 */
    Profile p = make_default_profile();
    p.speed = 88;
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, NULL) == 0);
    ASSERT(strstr(cmd, "rate=") != NULL);
}

TEST(test_build_command_fast_rate)
{
    /* 350 wpm = 2x 175 wpm → rate=2.00 */
    Profile p = make_default_profile();
    p.speed = 350;
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, NULL) == 0);
    ASSERT(strstr(cmd, "2.00") != NULL);
}

TEST(test_build_command_volume_present)
{
    Profile p = make_default_profile();
    p.volume = 200;
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, NULL) == 0);
    /* volume 200 → sapi 100 */
    ASSERT(strstr(cmd, "Volume=100") != NULL);
}

TEST(test_build_command_xml_special_in_text)
{
    /* Text containing XML special characters must produce a valid command */
    Profile p = make_default_profile();
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "cats & dogs < 3", &p, NULL) == 0);
    ASSERT(strstr(cmd, "&amp;")  != NULL);
    ASSERT(strstr(cmd, "&lt;")   != NULL);
    /* The raw & and < must NOT appear inside the SSML content */
    /* (we can only check the escaped forms exist; raw forms may appear in cmd boilerplate) */
}

TEST(test_build_command_apostrophe_in_text)
{
    /* Apostrophe in text must be doubled for PS single-quoted string */
    Profile p = make_default_profile();
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "I Don't Understand", &p, NULL) == 0);
    ASSERT(strstr(cmd, "Don''t") != NULL);
}

TEST(test_build_command_no_percent_in_rate)
{
    /* Rate must never use '%' — cmd.exe would expand it as an env-var */
    Profile p = make_default_profile();
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, NULL) == 0);
    ASSERT(strstr(cmd, "%") == NULL);
}

TEST(test_build_command_has_xml_lang)
{
    /* SAPI SpeakSsml() requires xml:lang on <speak> element */
    Profile p = make_default_profile();
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, NULL) == 0);
    ASSERT(strstr(cmd, "xml:lang") != NULL);
}

TEST(test_build_command_live_speech_uses_temp_wav)
{
    /* Live-speech path must route through temp file + SoundPlayer, not
     * SetOutputToDefaultAudioDevice (which fails with error 0x2 in subprocesses) */
    Profile p = make_default_profile();
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, NULL) == 0);
    ASSERT(strstr(cmd, "GetRandomFileName") != NULL);  /* leak-free temp path */
    ASSERT(strstr(cmd, "SoundPlayer")       != NULL);
    ASSERT(strstr(cmd, "PlaySync")        != NULL);
    ASSERT(strstr(cmd, "Remove-Item")     != NULL);
}

TEST(test_build_command_export_does_not_play_back)
{
    /* Export path must write to file only — no SoundPlayer / temp file */
    Profile p = make_default_profile();
    char cmd[2048];
    ASSERT(speech_build_command(cmd, sizeof(cmd), "Test", &p, "out.wav") == 0);
    ASSERT(strstr(cmd, "SoundPlayer")     == NULL);
    ASSERT(strstr(cmd, "GetTempFileName") == NULL);
}

/* -------------------------------------------------------------------------
 * speech_build_command: invalid input
 * ---------------------------------------------------------------------- */

TEST(test_build_command_null_buf)
{
    Profile p = make_default_profile();
    ASSERT(speech_build_command(NULL, 512, "Hello", &p, NULL) == -1);
}

/* -------------------------------------------------------------------------
 * SAPI audio integration test
 *
 * Calls speak_text() and verifies system() returns 0 (success).
 * This test DOES produce audible output — intentional.
 * The pipeline: SSML -> temp WAV (SAPI) -> SoundPlayer -> delete temp.
 * ---------------------------------------------------------------------- */

TEST(test_speak_text_sapi_audio_output)
{
    Profile p = make_default_profile();
    /* Short phrase; neutral pitch and default speed so it completes quickly */
    int rc = speak_text("OpenVoice test.", &p);
    ASSERT(rc == 0);
}

TEST(test_speak_text_with_adjusted_pitch)
{
    /* Higher pitch — verifies prosody parameter reaches SAPI without error */
    Profile p = make_default_profile();
    p.pitch = 5;
    int rc = speak_text("Higher pitch.", &p);
    ASSERT(rc == 0);
}

TEST(test_speak_text_with_adjusted_speed)
{
    /* Faster speed */
    Profile p = make_default_profile();
    p.speed = 250;
    int rc = speak_text("Faster speech.", &p);
    ASSERT(rc == 0);
}

TEST(test_speak_text_null_text)
{
    Profile p = make_default_profile();
    ASSERT(speak_text(NULL, &p) == -1);
}

TEST(test_speak_text_null_profile)
{
    ASSERT(speak_text("Hello", NULL) == -1);
}

TEST(test_speak_text_aac_phrase_yes)
{
    Profile p = make_default_profile();
    ASSERT(speak_text("Yes", &p) == 0);
}

TEST(test_speak_text_aac_phrase_with_apostrophe)
{
    /* "I Don't Understand" contains a single quote that must be escaped */
    Profile p = make_default_profile();
    ASSERT(speak_text("I Don't Understand", &p) == 0);
}

TEST(test_speak_text_aac_phrase_call_911)
{
    Profile p = make_default_profile();
    ASSERT(speak_text("Call 911", &p) == 0);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */
int main(void)
{
    printf("\n=== Speech Module Tests ===\n\n");

    printf("  -- speech_escape_text: valid input --\n");
    RUN(test_escape_no_special_chars);
    RUN(test_escape_single_quote);
    RUN(test_escape_multiple_quotes);
    RUN(test_escape_empty_string);
    RUN(test_escape_text_digits_unchanged);
    RUN(test_escape_text_all_ascii_printable);
    RUN(test_escape_text_preserves_xml_entities);

    printf("\n  -- speech_escape_text: invalid input --\n");
    RUN(test_escape_buffer_too_small);
    RUN(test_escape_text_null_buf);
    RUN(test_escape_text_null_text);
    RUN(test_escape_text_zero_bufsize);

    printf("\n  -- speech_escape_xml: valid input --\n");
    RUN(test_escape_xml_no_special);
    RUN(test_escape_xml_ampersand);
    RUN(test_escape_xml_angle_brackets);
    RUN(test_escape_xml_empty);
    RUN(test_escape_xml_combined);
    RUN(test_escape_xml_repeated_ampersand);

    printf("\n  -- speech_escape_xml: invalid input --\n");
    RUN(test_escape_xml_buffer_too_small);
    RUN(test_escape_xml_null_buf);
    RUN(test_escape_xml_null_text);
    RUN(test_escape_xml_zero_bufsize);

    printf("\n  -- speech_build_command: valid input --\n");
    RUN(test_build_command_basic);
    RUN(test_build_command_with_wav_output);
    RUN(test_build_command_language_flag);
    RUN(test_build_command_rate_reflects_speed);
    RUN(test_build_command_pitch_in_ssml);
    RUN(test_build_command_positive_pitch);
    RUN(test_build_command_negative_pitch);
    RUN(test_build_command_zero_pitch);
    RUN(test_build_command_slow_rate);
    RUN(test_build_command_fast_rate);
    RUN(test_build_command_volume_present);
    RUN(test_build_command_xml_special_in_text);
    RUN(test_build_command_apostrophe_in_text);
    RUN(test_build_command_no_percent_in_rate);
    RUN(test_build_command_has_xml_lang);
    RUN(test_build_command_live_speech_uses_temp_wav);
    RUN(test_build_command_export_does_not_play_back);

    printf("\n  -- speech_build_command: invalid input --\n");
    RUN(test_build_command_buffer_too_small);
    RUN(test_build_command_null_text);
    RUN(test_build_command_null_profile);
    RUN(test_build_command_null_buf);

    printf("\n  -- SAPI audio output (plays sound) --\n");
    RUN(test_speak_text_sapi_audio_output);
    RUN(test_speak_text_with_adjusted_pitch);
    RUN(test_speak_text_with_adjusted_speed);
    RUN(test_speak_text_aac_phrase_yes);
    RUN(test_speak_text_aac_phrase_with_apostrophe);
    RUN(test_speak_text_aac_phrase_call_911);

    printf("\n  -- speak_text: invalid input --\n");
    RUN(test_speak_text_null_text);
    RUN(test_speak_text_null_profile);

    printf("\n--- Results: %d/%d passed", tests_passed, tests_run);
    if (tests_failed > 0) {
        printf("  (%d FAILED)", tests_failed);
    }
    printf(" ---\n\n");

    return (tests_failed > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
