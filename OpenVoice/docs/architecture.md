# OpenVoice — Architecture Document

## 1. Overview

OpenVoice is a modular, single-process C99 application structured around a
clear separation of concerns.  Five independent modules are orchestrated by
`main.c` through a well-defined header API:

```
main.c
  ├── ui.c      ← ncurses presentation layer
  ├── speech.c  ← eSpeak NG TTS wrapper
  ├── phrase.c  ← phrase list management
  ├── profile.c ← voice settings persistence
  └── export.c  ← WAV file export
```

---

## 2. Module Descriptions

### 2.1 `ui` — Presentation Layer (`src/ui.c`, `include/ui.h`)

Owns all ncurses state.  Exposes a pure function interface: callers pass
data in and receive user selections back; the UI layer never stores
application state.

**Responsibilities:**
- Initialise / tear down ncurses (`ui_init`, `ui_cleanup`)
- Render the main menu and return the selected item
- Render the phrase board and return the selected phrase index
- Render the voice-settings slider panel and mutate the `Profile` in-place
- Provide a text-input dialogue box
- Provide modal message boxes and full-screen info screens

**Colour scheme:**  High-contrast pairs on a dark-blue background chosen
for accessibility (WCAG AA contrast ratios).

### 2.2 `speech` — TTS Wrapper (`src/speech.c`, `include/speech.h`)

Wraps `espeak-ng` via `system(3)`.  Uses single-quote shell escaping so that
arbitrary user-typed text is safe to embed in a shell command without a
shell-injection vulnerability.

**Key functions:**
- `speech_escape_text` — escapes single-quotes for shell embedding
- `speech_build_command` — constructs the complete `espeak-ng` command string
- `speak_text` — builds command and calls `system(3)` for live playback

### 2.3 `phrase` — Phrase Management (`src/phrase.c`, `include/phrase.h`)

Manages a `PhraseList` of up to `MAX_PHRASES` (64) entries.  Reads a plain-text
file (`data/phrases.txt`); provides a built-in fallback for when the file is
absent.

**Key functions:**
- `phrase_load` — parse `phrases.txt` (skips `#` comments, blank lines)
- `phrase_load_defaults` — populate with 20 built-in AAC phrases
- `phrase_add` — append a single phrase; returns `-1` if list is full
- `phrase_list_free` — zero the struct (forward-compatibility stub)

### 2.4 `profile` — Settings Persistence (`src/profile.c`, `include/profile.h`)

Saves and loads a `Profile` struct (pitch / speed / volume / language) to a
`key=value` text file that is human-editable without the application.

**Key functions:**
- `profile_set_defaults` — fill with eSpeak NG recommended defaults
- `profile_load` — parse `data/profile.cfg`; unknown keys are ignored
- `profile_save` — write with inline documentation comments
- `profile_clamp` — enforce parameter ranges after load or UI edit

### 2.5 `export` — WAV Export (`src/export.c`, `include/export.h`)

Thin wrapper around `speech_build_command` that passes a `-w <path>` flag
to eSpeak NG.

**Key functions:**
- `export_wav` — synthesise to a caller-supplied path
- `export_wav_with_timestamp` — generate a timestamped filename automatically

---

## 3. Data Flow

```
User Input (keyboard)
        │
        ▼
   ui_main_menu()
        │
        ├─── MENU_TYPE_MESSAGE ──────► ui_get_text_input()
        │                                     │
        │                                     ▼
        │                             speak_text(text, profile)
        │                                     │
        │                                     ▼
        │                         speech_build_command()
        │                                     │
        │                                     ▼
        │                             system("espeak-ng ...")
        │
        ├─── MENU_QUICK_PHRASES ─────► phrase_load()
        │                                     │
        │                                     ▼
        │                             ui_phrase_board()
        │                                     │
        │                                     ▼
        │                             speak_text(phrase, profile)
        │
        ├─── MENU_VOICE_SETTINGS ────► ui_voice_settings(profile)
        │                                     │
        │                                     ▼
        │                             profile_save(profile)
        │
        ├─── MENU_EXPORT_WAV ────────► ui_get_text_input()
        │                                     │
        │                                     ▼
        │                             export_wav(text, path, profile)
        │
        └─── MENU_EXIT ──────────────► profile_save() → ui_cleanup() → exit
```

---

## 4. File I/O

| File | Read | Write | Format |
|---|---|---|---|
| `data/phrases.txt` | At phrase board open | Never (user edits manually) | Plain text, one phrase per line |
| `data/profile.cfg` | At startup | On exit and after settings save | `key=value` pairs |
| `output.wav` | Never | On WAV export | PCM WAV (written by eSpeak NG) |

---

## 5. Error Handling Strategy

- Every function that can fail returns `int` (0 = success, -1 = error).
- On file-open failure, the application falls back to built-in defaults
  rather than aborting.
- Shell commands are validated before `system(3)` is called; if the command
  string is too long to fit in the buffer, the call is cancelled and an error
  is returned.
- ncurses errors are non-fatal; the UI degrades gracefully on small terminals.

---

## 6. Concurrency

OpenVoice is single-threaded.  Speech synthesis runs synchronously via
`system(3)`, which blocks the UI during playback.  A future improvement
would fork a child process for synthesis so the UI remains responsive.

---

## 7. Portability

| Component | Linux | macOS | Windows |
|---|---|---|---|
| C99 core logic | ✓ | ✓ | ✓ (with MinGW) |
| ncurses | ✓ | ✓ (pdcurses) | ✓ (pdcurses) |
| eSpeak NG | ✓ | ✓ | ✓ |
| `system(3)` shell escaping | POSIX | POSIX | Needs `cmd.exe` adaptation |
