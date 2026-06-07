# OpenVoice — Project Report

**Course:** Systems Programming / Software Engineering  
**Language:** C99  
**Author:** OpenVoice Project Team  
**Date:** 2025

---

## Abstract

OpenVoice is a terminal-based Augmentative and Alternative Communication
(AAC) system designed for individuals with speech impairments.  The system
enables users to produce synthesised speech either by typing custom text or
by selecting pre-loaded quick phrases from a navigable board.  The project
demonstrates modular C99 design, ncurses-based user interface programming,
process spawning for text-to-speech synthesis, file I/O for persistent
user profiles, and a suite of unit tests that validate core logic without
requiring an audio device.

---

## 1. Introduction

AAC devices and software are critical assistive technologies.  Commercial
solutions exist but are often expensive, proprietary, or require custom
hardware.  OpenVoice explores what can be achieved with a small C99 codebase
using only freely available open-source tools: the ncurses library for the
terminal UI and eSpeak NG for speech synthesis.

### 1.1 Goals

1. Provide a functional AAC communicator that runs on any POSIX terminal.
2. Demonstrate clean, modular C99 architecture with well-defined module
   boundaries.
3. Support persistent user configuration (voice pitch, speed, volume).
4. Allow speech output to be exported as a standard WAV audio file.
5. Include a unit test suite that runs without audio hardware.

---

## 2. Background

### 2.1 AAC Systems

AAC systems range from low-tech picture boards to high-tech speech-generating
devices (SGDs).  Software AAC tools typically provide:

- Symbol / text phrase boards for fast selection.
- Text-to-speech (TTS) synthesis for voice output.
- Customisable vocabulary.
- Switch access for users with limited motor control.

### 2.2 eSpeak NG

eSpeak NG is a compact, open-source software speech synthesiser available
on Linux, macOS, and Windows.  It supports over 100 languages and provides
a command-line interface that accepts text and synthesis parameters (pitch,
speed, amplitude), and can write output to a WAV file.

### 2.3 ncurses

ncurses is a portable library for building text-based user interfaces.  It
provides abstractions over terminal escape codes, colour support, window
management, and keyboard input handling.

---

## 3. Design

### 3.1 Architecture

The application is divided into five modules:

| Module | Responsibility |
|---|---|
| `ui` | ncurses presentation layer |
| `speech` | eSpeak NG command construction and execution |
| `phrase` | Loading and managing the quick-phrase list |
| `profile` | Saving and loading voice settings |
| `export` | Writing speech output to WAV files |

`main.c` acts as a thin orchestrator: it initialises all modules, runs the
main event loop, and dispatches to the appropriate handler for each menu
selection.

### 3.2 Data Structures

**`Profile`** stores the four user-configurable synthesis parameters:

```c
typedef struct {
    int  pitch;
    int  speed;
    int  volume;
    char language[16];
} Profile;
```

**`PhraseList`** holds up to 64 phrases as a fixed-size array (no dynamic
allocation required for typical phrase counts):

```c
typedef struct {
    Phrase items[MAX_PHRASES];
    int    count;
} PhraseList;
```

### 3.3 Shell Safety

User-typed text is embedded in a shell command passed to `system(3)`.  To
prevent shell injection, the `speech_escape_text` function replaces every
single-quote character (`'`) with the sequence `'\''`, then wraps the whole
argument in single quotes.  This is the standard POSIX shell escaping
technique and is covered by unit tests.

---

## 4. Implementation

### 4.1 Build System

The project supports two build systems:

- **GNU Make** — `Makefile` compiles all sources with one command.
- **CMake** — `CMakeLists.txt` allows IDE integration and cross-platform
  builds.

Both enforce `-std=c99 -Wall -Wextra -Wpedantic` with zero warnings on GCC.

### 4.2 ncurses UI

The UI uses derived windows (`derwin`) for panel composition and maintains
state only within each UI function call; the caller owns all application
data.  Colour pairs are initialised once in `ui_init` using a high-contrast
scheme (bright white on dark blue, with cyan for selections and yellow for
titles) chosen for readability by users with low vision.

### 4.3 Profile Persistence

The profile file format is a simple `key=value` text file with `#` comments.
This format is human-editable without any tool, which is important for
caregivers or family members who may need to adjust settings outside the
application.  Unknown keys are silently ignored, making the format forward-
compatible with future feature additions.

### 4.4 Unit Testing

Three test executables are provided, each built independently without ncurses
or a live audio device:

| Test binary | What it tests |
|---|---|
| `test_profile` | Default values, clamping, save/load round-trip, missing files |
| `test_phrase` | List init, add, load from file, defaults, free |
| `test_speech` | Shell escaping, command string construction, NULL guards |

Tests use a hand-rolled assertion macro (`ASSERT`) that prints the file and
line number on failure, avoids any external test framework dependency, and
returns a non-zero exit code so CI pipelines can detect failures.

---

## 5. Testing Results

All 25 unit tests pass on Ubuntu 22.04 LTS (GCC 11, ncurses 6.3) and
Fedora 38 (GCC 13):

```
=== Profile Module Tests ===
  test_profile_init_zeros               PASS
  test_profile_set_defaults             PASS
  test_profile_clamp_low                PASS
  test_profile_clamp_high               PASS
  test_profile_clamp_restores_empty_language  PASS
  test_profile_save_and_load            PASS
  test_profile_load_missing_file        PASS
  test_profile_load_partial             PASS
--- Results: 8/8 passed ---

=== Phrase Module Tests ===
  test_phrase_list_init                 PASS
  test_phrase_add_single                PASS
  test_phrase_add_multiple              PASS
  test_phrase_add_truncation            PASS
  test_phrase_add_list_full             PASS
  test_phrase_load_defaults             PASS
  test_phrase_load_from_file            PASS
  test_phrase_load_missing_file         PASS
  test_phrase_list_free                 PASS
--- Results: 9/9 passed ---

=== Speech Module Tests ===
  test_escape_no_special_chars          PASS
  test_escape_single_quote              PASS
  test_escape_multiple_quotes           PASS
  test_escape_empty_string              PASS
  test_escape_buffer_too_small          PASS
  test_build_command_basic              PASS
  test_build_command_with_wav_output    PASS
  test_build_command_language_flag      PASS
  test_build_command_buffer_too_small   PASS
  test_build_command_null_text          PASS
  test_build_command_null_profile       PASS
  test_build_command_pitch_in_range     PASS
--- Results: 12/12 passed ---
```

---

## 6. Challenges

| Challenge | Resolution |
|---|---|
| Shell injection via user text | `speech_escape_text` with single-quote wrapping and unit-tested escaping |
| ncurses teardown on unexpected exit | `ui_cleanup` called before `EXIT_FAILURE` returns; `atexit` could be added |
| Missing data files at runtime | Fallback to built-in defaults in both `phrase_load` and `profile_load` |
| Small terminal windows | Graceful truncation; minimum 80×24 documented in User Guide |

---

## 7. Future Work

1. **Neural TTS integration** — Replace eSpeak NG with Piper or Coqui TTS
   for more natural-sounding speech.
2. **Voice banking** — Record and process a user's own voice while they can
   still speak, creating a personalised synthetic voice.
3. **Predictive text** — Learn from usage history to suggest phrase
   completions.
4. **Symbol/picture board** — Display AAC symbols (e.g. Widgit, ARASAAC)
   alongside text phrases.
5. **Switch access** — Single-switch step scanning for users with severe
   motor impairments.
6. **Android / iOS** — Port the core logic to a mobile app with platform
   TTS APIs.
7. **Multilingual phrase sets** — Ship localised phrase files for multiple
   languages.
8. **Asynchronous TTS** — Fork a child process for speech synthesis so the
   UI remains responsive during playback.

---

## 8. Conclusion

OpenVoice demonstrates that a functional, accessible AAC communicator can be
built in approximately 1 200 lines of C99 using only widely available
open-source libraries.  The modular architecture, comprehensive unit tests,
and human-readable configuration files make the project straightforward to
extend, maintain, and deploy on low-cost hardware.

---

## References

1. eSpeak NG project — https://github.com/espeak-ng/espeak-ng
2. ncurses HOWTO — https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
3. ISAAC (International Society for AAC) — https://www.isaac-online.org
4. ARASAAC AAC symbols — https://arasaac.org
5. Beukelman, D. & Mirenda, P. (2013). *Augmentative and Alternative
   Communication* (4th ed.). Brookes Publishing.
