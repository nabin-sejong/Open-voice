/*
 * OpenVoice AAC Communicator
 * include/speech.h - Text-to-Speech API
 *
 * Wraps eSpeak NG command-line invocations.
 *
 * Standard: C99
 */

#ifndef OPENVOICE_SPEECH_H
#define OPENVOICE_SPEECH_H

#include "profile.h"

/* -------------------------------------------------------------------------
 * eSpeak NG binary name (resolved via PATH)
 * ---------------------------------------------------------------------- */
#define ESPEAK_BINARY  "espeak-ng"

/* -------------------------------------------------------------------------
 * Speech functions
 * ---------------------------------------------------------------------- */

/**
 * speak_text - synthesise and play speech through the default audio device.
 *
 * Builds an espeak-ng command line using the pitch, speed, and volume
 * values stored in @p profile, then executes it via system(3).
 *
 * @param text     NUL-terminated text to speak.
 * @param profile  Voice profile containing synthesis parameters.
 * Returns 0 on success, non-zero on error.
 */
int speak_text(const char *text, const Profile *profile);

/**
 * speech_build_command - build an espeak-ng command string.
 *
 * Writes a ready-to-use shell command into @p buf.  Exported so that
 * unit tests can validate the command string without invoking the shell.
 *
 * @param buf      Destination buffer.
 * @param bufsize  Size of buf in bytes.
 * @param text     Text to synthesise.
 * @param profile  Voice profile.
 * @param wav_out  If non-NULL, adds "-w <wav_out>" to write a WAV file
 *                 instead of playing audio live.
 * Returns 0 on success, -1 if buf is too small.
 */
int speech_build_command(char *buf, int bufsize,
                         const char *text,
                         const Profile *profile,
                         const char *wav_out);

/**
 * speech_escape_text - copy text into buf, escaping single-quotes so the
 * string is safe to embed inside a single-quoted shell argument.
 *
 * @param buf      Destination buffer.
 * @param bufsize  Size of buf.
 * @param text     Source text.
 * Returns 0 on success, -1 if buf is too small.
 */
int speech_escape_text(char *buf, int bufsize, const char *text);

#endif /* OPENVOICE_SPEECH_H */
