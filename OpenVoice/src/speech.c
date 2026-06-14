/*
 * OpenVoice AAC Communicator
 * src/speech.c - Text-to-Speech via Windows SAPI + SSML
 *
 * Uses PowerShell's System.Speech.Synthesis.SpeechSynthesizer with
 * SpeakSsml() so that SSML <prosody> attributes (pitch in semitones,
 * rate as a ratio) are honoured — no external TTS install needed.
 *
 * Live-speech pipeline (wav_out = NULL)
 * --------------------------------------
 * SAPI's direct audio output fails with error 0x2 on many Windows
 * configurations when spawned from a subprocess.  The workaround:
 *   1. Synthesise SSML to a unique temp WAV path (no GetTempFileName
 *      leak — we use GetTempPath + GetRandomFileName instead).
 *   2. Play the WAV with System.Media.SoundPlayer.PlaySync().
 *      System.Windows.Forms is loaded explicitly so SoundPlayer is
 *      always available regardless of PowerShell's default assembly set.
 *   3. Delete the temp file.
 * This is transparent to the caller.
 *
 * Export pipeline (wav_out != NULL)
 * ----------------------------------
 * Synthesise directly to the caller-supplied WAV path — no playback.
 *
 * SSML quoting strategy
 * ----------------------
 * The PowerShell -Command argument is wrapped in outer double-quotes by
 * cmd.exe.  To avoid ANY double-quote inside that wrapper (which would
 * confuse cmd.exe), the SSML uses single-quoted XML attributes.  Inside
 * the PowerShell $ssml='...' assignment, every literal ' is pre-doubled
 * to '' (PowerShell single-quoted string escaping).
 * The % character is never used (rate expressed as a decimal ratio) to
 * prevent cmd.exe from attempting environment-variable expansion.
 *
 * Console mode
 * ------------
 * The TUI sets the console input mode to ENABLE_WINDOW_INPUT |
 * ENABLE_MOUSE_INPUT (dropping ENABLE_PROCESSED_INPUT).  PowerShell
 * spawned via system() inherits that modified mode and can silently
 * fail its startup, causing audio not to play.  speak_text() saves
 * and restores the original console mode around the system() call.
 *
 * Standard: C99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "../include/speech.h"
#include "../include/profile.h"

/* -------------------------------------------------------------------------
 * speech_escape_xml
 *
 * Replaces XML special characters so text is safe inside SSML element
 * content.  Apply this BEFORE speech_escape_text.
 *   &  →  &amp;    <  →  &lt;    >  →  &gt;
 * ---------------------------------------------------------------------- */
int speech_escape_xml(char *buf, int bufsize, const char *text)
{
    if (!buf || bufsize <= 0 || !text) return -1;

    static const struct { char ch; const char *entity; int elen; } map[] = {
        { '&', "&amp;", 5 },
        { '<', "&lt;",  4 },
        { '>', "&gt;",  4 },
    };

    int out = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        int replaced = 0;
        for (int m = 0; m < 3; m++) {
            if (text[i] == map[m].ch) {
                if (out + map[m].elen >= bufsize) return -1;
                memcpy(buf + out, map[m].entity, (size_t)map[m].elen);
                out += map[m].elen;
                replaced = 1;
                break;
            }
        }
        if (!replaced) {
            if (out + 1 >= bufsize) return -1;
            buf[out++] = text[i];
        }
    }
    buf[out] = '\0';
    return 0;
}

/* -------------------------------------------------------------------------
 * speech_escape_text
 *
 * Doubles single-quotes for embedding inside a PowerShell single-quoted
 * string.  Apply AFTER speech_escape_xml (XML entities contain no ').
 *   '  →  ''
 * ---------------------------------------------------------------------- */
int speech_escape_text(char *buf, int bufsize, const char *text)
{
    if (!buf || bufsize <= 0 || !text) return -1;

    int out = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\'') {
            if (out + 2 >= bufsize) return -1;
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
 * Internal helpers
 * ---------------------------------------------------------------------- */

/* volume (0-200, default 100) → SAPI Volume (0-100) */
static int volume_to_sapi(int volume)
{
    int v = volume / 2;
    if (v <   0) v =   0;
    if (v > 100) v = 100;
    return v;
}

/* speed (80-450 wpm, default 175) → SSML rate as a decimal ratio.
 * Uses a plain number (no %) to prevent cmd.exe env-var expansion.
 * SSML spec allows both "100%" and "1.00" as equivalent rate values. */
static void speed_to_ssml_rate(char *buf, int bufsize, int speed)
{
    double ratio = speed / 175.0;
    if (ratio < 0.20) ratio = 0.20;
    if (ratio > 3.00) ratio = 3.00;
    snprintf(buf, (size_t)bufsize, "%.2f", ratio);
}

/* -------------------------------------------------------------------------
 * speech_build_command
 *
 * Builds a complete PowerShell command string for either live speech or
 * WAV export, using SSML <prosody> for pitch and rate control.
 *
 * SSML skeleton (single-quoted XML attributes, ' pre-doubled for PS):
 *   <speak version='1.0'
 *          xmlns='http://www.w3.org/2001/10/synthesis'
 *          xml:lang='LANG'>
 *     <prosody pitch='+Xst' rate='R.RR'>TEXT</prosody>
 *   </speak>
 * ---------------------------------------------------------------------- */
int speech_build_command(char *buf, int bufsize,
                         const char *text,
                         const Profile *profile,
                         const char *wav_out)
{
    if (!buf || bufsize <= 0 || !text || !profile) return -1;

    /* Step 1: XML-escape the text (&, <, >) */
    char xml_text[4096];
    if (speech_escape_xml(xml_text, sizeof(xml_text), text) != 0) {
        strncpy(xml_text, "text too long", sizeof(xml_text) - 1);
        xml_text[sizeof(xml_text) - 1] = '\0';
    }

    /* Step 2: PS-escape the XML-escaped text (double ') */
    char safe_text[8193];
    if (speech_escape_text(safe_text, sizeof(safe_text), xml_text) != 0) {
        strncpy(safe_text, "text too long", sizeof(safe_text) - 1);
        safe_text[sizeof(safe_text) - 1] = '\0';
    }

    /* SSML prosody parameters */
    int  pitch_st = profile->pitch;   /* -10 to +10 semitones */
    char rate_str[16];
    speed_to_ssml_rate(rate_str, sizeof(rate_str), profile->speed);
    int  vol = volume_to_sapi(profile->volume);

    /* Language tag (e.g. "en-us") — no special chars expected */
    const char *lang = profile->language[0] ? profile->language : "en-US";

    int written;

    if (wav_out) {
        /* ---- Export path: write directly to caller-supplied WAV ---- */
        char safe_wav[512];
        if (speech_escape_text(safe_wav, sizeof(safe_wav), wav_out) != 0) {
            strncpy(safe_wav, "output.wav", sizeof(safe_wav) - 1);
            safe_wav[sizeof(safe_wav) - 1] = '\0';
        }
        written = snprintf(buf, (size_t)bufsize,
            "powershell -NoProfile -Command \""
            "Add-Type -AssemblyName System.Speech;"
            "$ssml='<speak version=''1.0''"
                   " xmlns=''http://www.w3.org/2001/10/synthesis''"
                   " xml:lang=''%s''>"
                   "<prosody pitch=''%+dst'' rate=''%s''>%s</prosody>"
            "</speak>';"
            "$s=New-Object System.Speech.Synthesis.SpeechSynthesizer;"
            "$s.Volume=%d;"
            "$s.SetOutputToWaveFile('%s');"
            "$s.SpeakSsml($ssml);"
            "$s.Dispose()\"",
            lang, pitch_st, rate_str, safe_text,
            vol, safe_wav);
    } else {
        /*
         * ---- Live-speech path ----------------------------------------
         * SAPI direct audio (SetOutputToDefaultAudioDevice + Speak) fails
         * with error 0x2 in many subprocess contexts.  Workaround:
         *   1. Build a unique temp WAV path with GetTempPath +
         *      GetRandomFileName (avoids the GetTempFileName leak where
         *      it creates a .tmp file that never gets cleaned up).
         *   2. Synthesise to that temp WAV.
         *   3. Play it with System.Media.SoundPlayer.PlaySync().
         *      System.Windows.Forms is loaded explicitly — SoundPlayer
         *      lives in that assembly and is NOT auto-loaded in a
         *      -NoProfile subprocess.
         *   4. Delete the temp WAV (SilentlyContinue so failure is safe).
         */
        written = snprintf(buf, (size_t)bufsize,
            "powershell -NoProfile -Command \""
            "Add-Type -AssemblyName System.Speech;"
            "Add-Type -AssemblyName System.Windows.Forms;"
            "$ssml='<speak version=''1.0''"
                   " xmlns=''http://www.w3.org/2001/10/synthesis''"
                   " xml:lang=''%s''>"
                   "<prosody pitch=''%+dst'' rate=''%s''>%s</prosody>"
            "</speak>';"
            "$tmp=[System.IO.Path]::Combine("
                  "[System.IO.Path]::GetTempPath(),"
                  "[System.IO.Path]::ChangeExtension("
                      "[System.IO.Path]::GetRandomFileName(),'.wav'));"
            "$s=New-Object System.Speech.Synthesis.SpeechSynthesizer;"
            "$s.Volume=%d;"
            "$s.SetOutputToWaveFile($tmp);"
            "$s.SpeakSsml($ssml);"
            "$s.Dispose();"
            "(New-Object System.Media.SoundPlayer $tmp).PlaySync();"
            "Remove-Item $tmp -ErrorAction SilentlyContinue\"",
            lang, pitch_st, rate_str, safe_text,
            vol);
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

    char cmd[8192];
    if (speech_build_command(cmd, sizeof(cmd), text, profile, NULL) != 0) {
        fprintf(stderr, "speak_text: failed to build command\n");
        return -1;
    }

    /*
     * The TUI sets the console input mode to ENABLE_WINDOW_INPUT |
     * ENABLE_MOUSE_INPUT, dropping ENABLE_PROCESSED_INPUT.  When
     * system() spawns cmd.exe → powershell, the child inherits those
     * handles in the same modified mode.  PowerShell's startup code
     * then silently fails — it cannot interact with the console
     * normally — and audio does not play.
     *
     * Fix: save the current console mode, restore a normal interactive
     * mode for the subprocess, then re-apply the saved TUI mode after.
     */
#ifdef _WIN32
    HANDLE hIn       = GetStdHandle(STD_INPUT_HANDLE);
    DWORD  saved_mode = 0;
    BOOL   have_mode  = GetConsoleMode(hIn, &saved_mode);
    if (have_mode) {
        SetConsoleMode(hIn,
            ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT |
            ENABLE_ECHO_INPUT      | ENABLE_EXTENDED_FLAGS);
    }
#endif

    int ret = system(cmd);

#ifdef _WIN32
    /* Restore TUI mode whether or not speech succeeded */
    if (have_mode) {
        SetConsoleMode(hIn, saved_mode);
    }
#endif

    if (ret != 0) {
        fprintf(stderr, "speak_text: system() returned %d\n", ret);
        return -1;
    }
    return 0;
}

