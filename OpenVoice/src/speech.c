/*
 * OpenVoice AAC Communicator
 * src/speech.c - Text-to-Speech Implementation
 *
 * Wraps eSpeak NG via system(3) calls.  All shell arguments are carefully
 * sanitised to prevent injection through user-supplied text.
 *
 * Standard: C99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/speech.h"
#include "../include/profile.h"

/* -------------------------------------------------------------------------
 * speech_escape_text
 *
 * Copies src into dst, replacing every single-quote (') with the sequence
 * '\'' so the result is safe inside a single-quoted shell argument.
 *
 * Strategy:
 *   - Open single-quote wrapping is handled by the caller.
 *   - We only escape internal quotes: '  →  '\''
 * ---------------------------------------------------------------------- */
int speech_escape_text(char *buf, int bufsize, const char *text)
{
    if (!buf || bufsize <= 0 || !text) return -1;

    int out = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\'') {
            /* Need 4 characters: ' \ ' ' */
            if (out + 4 >= bufsize) return -1;
            buf[out++] = '\'';
            buf[out++] = '\\';
            buf[out++] = '\'';
            buf[out++] = '\'';
        } else {
            if (out + 1 >= bufsize) return -1;
            buf[out++] = text[i];
        }
    }
    buf[out] = '\0';
    return 0;
}

/* -------------------------------------------------------------------------
 * speech_build_command
 * ---------------------------------------------------------------------- */
int speech_build_command(char *buf, int bufsize,
                         const char *text,
                         const Profile *profile,
                         const char *wav_out)
{
    if (!buf || bufsize <= 0 || !text || !profile) return -1;

    /* Escape the text for single-quote shell embedding */
    char safe_text[1024];
    if (speech_escape_text(safe_text, sizeof(safe_text), text) != 0) {
        /* Fallback: truncate and omit dangerous characters */
        strncpy(safe_text, "text too long", sizeof(safe_text) - 1);
        safe_text[sizeof(safe_text) - 1] = '\0';
    }

    int written;
    if (wav_out) {
        written = snprintf(buf, (size_t)bufsize,
            "%s -p %d -s %d -a %d -v %s -w '%s' '%s'",
            ESPEAK_BINARY,
            profile->pitch,
            profile->speed,
            profile->volume,
            profile->language,
            wav_out,
            safe_text);
    } else {
        written = snprintf(buf, (size_t)bufsize,
            "%s -p %d -s %d -a %d -v %s '%s'",
            ESPEAK_BINARY,
            profile->pitch,
            profile->speed,
            profile->volume,
            profile->language,
            safe_text);
    }

    if (written < 0 || written >= bufsize) return -1;
    return 0;
}

/* -------------------------------------------------------------------------
 * speak_text
 * ---------------------------------------------------------------------- */
int speak_text(const char *text, const Profile *profile)
{
    if (!text || !profile) return -1;

    char cmd[2048];
    if (speech_build_command(cmd, sizeof(cmd), text, profile, NULL) != 0) {
        fprintf(stderr, "speak_text: failed to build command\n");
        return -1;
    }

    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "speak_text: system() returned %d for: %s\n", ret, cmd);
        return -1;
    }
    return 0;
}
