/*
 * OpenVoice AAC Communicator
 * include/export.h - WAV Export API
 *
 * Declares functions for exporting synthesised speech to WAV files.
 *
 * Standard: C99
 */

#ifndef OPENVOICE_EXPORT_H
#define OPENVOICE_EXPORT_H

#include "profile.h"

/* -------------------------------------------------------------------------
 * Functions
 * ---------------------------------------------------------------------- */

/**
 * export_wav - synthesise @p text and write the audio to a WAV file.
 *
 * Uses espeak-ng's built-in "-w" flag to produce a standard PCM WAV.
 *
 * @param text      NUL-terminated text to synthesise.
 * @param wav_path  Output file path (e.g. "output.wav").
 * @param profile   Voice profile (pitch / speed / volume / language).
 * Returns 0 on success, -1 on error.
 */
int export_wav(const char *text, const char *wav_path, const Profile *profile);

/**
 * export_wav_with_timestamp - like export_wav() but auto-generates a
 * timestamped filename of the form "openvoice_YYYYMMDD_HHMMSS.wav".
 *
 * The generated path is written into @p path_out (up to @p path_out_len
 * bytes including the NUL terminator).
 *
 * @param text          Text to synthesise.
 * @param path_out      Buffer to receive the generated filename.
 * @param path_out_len  Size of path_out.
 * @param profile       Voice profile.
 * Returns 0 on success, -1 on error.
 */
int export_wav_with_timestamp(const char *text,
                              char *path_out,
                              int   path_out_len,
                              const Profile *profile);

#endif /* OPENVOICE_EXPORT_H */
