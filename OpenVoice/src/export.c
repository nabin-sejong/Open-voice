/*
 * OpenVoice AAC Communicator
 * src/export.c - WAV Export Implementation
 *
 * Uses Windows SAPI (via PowerShell System.Speech) to write
 * synthesised speech to a WAV file.
 *
 * Standard: C99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/export.h"
#include "../include/speech.h"
#include "../include/profile.h"

/* -------------------------------------------------------------------------
 * export_wav
 * ---------------------------------------------------------------------- */
int export_wav(const char *text, const char *wav_path, const Profile *profile)
{
    if (!text || !wav_path || !profile) return -1;

    char cmd[2048];
    if (speech_build_command(cmd, sizeof(cmd), text, profile, wav_path) != 0) {
        fprintf(stderr, "export_wav: failed to build command\n");
        return -1;
    }

    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "export_wav: system() returned %d\n", ret);
        return -1;
    }
    return 0;
}

/* -------------------------------------------------------------------------
 * export_wav_with_timestamp
 * ---------------------------------------------------------------------- */
int export_wav_with_timestamp(const char *text,
                               char *path_out,
                               int   path_out_len,
                               const Profile *profile)
{
    if (!text || !path_out || path_out_len <= 0 || !profile) return -1;

    /* Build timestamped filename */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    int written = snprintf(path_out, (size_t)path_out_len,
                           "openvoice_%04d%02d%02d_%02d%02d%02d.wav",
                           t->tm_year + 1900,
                           t->tm_mon  + 1,
                           t->tm_mday,
                           t->tm_hour,
                           t->tm_min,
                           t->tm_sec);

    if (written < 0 || written >= path_out_len) {
        /* Buffer too small; use fallback name */
        strncpy(path_out, "output.wav", (size_t)path_out_len - 1);
        path_out[path_out_len - 1] = '\0';
    }

    return export_wav(text, path_out, profile);
}
