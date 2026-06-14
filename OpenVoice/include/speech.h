/*
 * OpenVoice AAC Communicator
 * include/speech.h - Text-to-Speech API
 *
 * Wraps Windows SAPI via PowerShell (no external TTS install required).
 *
 * Standard: C99
 */

#ifndef OPENVOICE_SPEECH_H
#define OPENVOICE_SPEECH_H

#include "profile.h"

/* -------------------------------------------------------------------------
 * Speech functions
 * ---------------------------------------------------------------------- */

/**
 * speak_text - synthesise and play speech through the default audio device.
 *
 * Builds a PowerShell SAPI command using the speed and volume values stored
 * in @p profile, then executes it via system(3).
 *
 * @param text     NUL-terminated text to speak.
 * @param profile  Voice profile containing synthesis parameters.
 * Returns 0 on success, non-zero on error.
 */
int speak_text(const char *text, const Profile *profile);

/**
 * speech_build_command - build a PowerShell SAPI command string.
 *
 * Writes a ready-to-use shell command into @p buf.  Exported so that
 * unit tests can validate the command string without invoking PowerShell.
 *
 * @param buf      Destination buffer.
 * @param bufsize  Size of buf in bytes.
 * @param text     Text to synthesise.
 * @param profile  Voice profile.
 * @param wav_out  If non-NULL, directs output to a WAV file instead of
 *                 the default audio device.
 * Returns 0 on success, -1 if buf is too small.
 */
int speech_build_command(char *buf, int bufsize,
                         const char *text,
                         const Profile *profile,
                         const char *wav_out);

/**
 * speech_escape_text - copy text into buf, doubling single-quotes so the
 * string is safe to embed inside a PowerShell single-quoted string.
 *   PowerShell rule:  '  →  ''
 *
 * @param buf      Destination buffer.
 * @param bufsize  Size of buf.
 * @param text     Source text.
 * Returns 0 on success, -1 if buf is too small.
 */
int speech_escape_text(char *buf, int bufsize, const char *text);

/**
 * speech_escape_xml - copy text into buf, replacing XML special characters
 * so the result is safe inside an SSML element's text content.
 *   &  →  &amp;    <  →  &lt;    >  →  &gt;
 *
 * Apply this BEFORE speech_escape_text when embedding text in SSML.
 *
 * @param buf      Destination buffer.
 * @param bufsize  Size of buf.
 * @param text     Source text.
 * Returns 0 on success, -1 if buf is too small.
 */
int speech_escape_xml(char *buf, int bufsize, const char *text);

#endif /* OPENVOICE_SPEECH_H */
