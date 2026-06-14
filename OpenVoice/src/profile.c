/*
 * OpenVoice AAC Communicator
 * src/profile.c - Voice Profile Implementation
 *
 * Saves and loads user voice settings (pitch, speed, volume, language) to a
 * simple key=value text file.
 *
 * Standard: C99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/profile.h"

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

/** Trim leading/trailing whitespace in-place. */
static void trim(char *s)
{
    if (!s) return;
    int len = (int)strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
    int start = 0;
    while (s[start] && isspace((unsigned char)s[start])) start++;
    if (start > 0) memmove(s, s + start, (size_t)(len - start + 1));
}

/** Clamp an integer to [lo, hi]. */
static int clamp_int(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* -------------------------------------------------------------------------
 * profile_init
 * ---------------------------------------------------------------------- */
void profile_init(Profile *p)
{
    if (!p) return;
    memset(p, 0, sizeof(*p));
}

/* -------------------------------------------------------------------------
 * profile_set_defaults
 * ---------------------------------------------------------------------- */
void profile_set_defaults(Profile *p)
{
    if (!p) return;
    p->pitch  = PITCH_DEF;
    p->speed  = SPEED_DEF;
    p->volume = VOLUME_DEF;
    strncpy(p->language, "en-us", LANG_TAG_LEN - 1);
    p->language[LANG_TAG_LEN - 1] = '\0';
}

/* -------------------------------------------------------------------------
 * profile_clamp
 * ---------------------------------------------------------------------- */
void profile_clamp(Profile *p)
{
    if (!p) return;
    p->pitch  = clamp_int(p->pitch,  PITCH_MIN,  PITCH_MAX);
    p->speed  = clamp_int(p->speed,  SPEED_MIN,  SPEED_MAX);
    p->volume = clamp_int(p->volume, VOLUME_MIN, VOLUME_MAX);

    /* Ensure language tag is not empty */
    if (p->language[0] == '\0') {
        strncpy(p->language, "en-us", LANG_TAG_LEN - 1);
        p->language[LANG_TAG_LEN - 1] = '\0';
    }
}

/* -------------------------------------------------------------------------
 * profile_load
 *
 * File format: lines of "key=value" pairs, '#' introduces a comment.
 * ---------------------------------------------------------------------- */
int profile_load(Profile *p, const char *path)
{
    if (!p || !path) return -1;

    /* Start with defaults so any missing key stays at a sane value */
    profile_set_defaults(p);

    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    char line[128];
    while (fgets(line, sizeof(line), fp)) {
        trim(line);
        if (line[0] == '#' || line[0] == '\0') continue;

        /* Split at '=' */
        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char *key = line;
        char *val = eq + 1;
        trim(key);
        trim(val);

        if (strcmp(key, "pitch") == 0) {
            p->pitch = atoi(val);
        } else if (strcmp(key, "speed") == 0) {
            p->speed = atoi(val);
        } else if (strcmp(key, "volume") == 0) {
            p->volume = atoi(val);
        } else if (strcmp(key, "language") == 0) {
            strncpy(p->language, val, LANG_TAG_LEN - 1);
            p->language[LANG_TAG_LEN - 1] = '\0';
        }
        /* Unknown keys are silently ignored */
    }

    fclose(fp);
    profile_clamp(p);
    return 0;
}

/* -------------------------------------------------------------------------
 * profile_save
 * ---------------------------------------------------------------------- */
int profile_save(const Profile *p, const char *path)
{
    if (!p || !path) return -1;

    FILE *fp = fopen(path, "w");
    if (!fp) return -1;

    fprintf(fp, "# OpenVoice Voice Profile\n");
    fprintf(fp, "# Edit this file or use the Voice Settings menu.\n");
    fprintf(fp, "#\n");
    fprintf(fp, "# pitch:    %d to %+d  (semitones, default %+d = neutral F0)\n",
            PITCH_MIN, PITCH_MAX, PITCH_DEF);
    fprintf(fp, "# speed:    %d - %d  (default %d, words per minute)\n",
            SPEED_MIN, SPEED_MAX, SPEED_DEF);
    fprintf(fp, "# volume:   %d - %d  (default %d)\n",
            VOLUME_MIN, VOLUME_MAX, VOLUME_DEF);
    fprintf(fp, "# language: BCP 47 language tag for SAPI voice (e.g. en-us, en-gb, fr, de)\n");
    fprintf(fp, "\n");
    fprintf(fp, "pitch=%d\n",    p->pitch);
    fprintf(fp, "speed=%d\n",    p->speed);
    fprintf(fp, "volume=%d\n",   p->volume);
    fprintf(fp, "language=%s\n", p->language);

    fclose(fp);
    return 0;
}

