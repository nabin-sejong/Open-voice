# OpenVoice — AAC Communicator

> **Augmentative and Alternative Communication for users with speech impairments**

OpenVoice is a free, open-source Windows AAC (Augmentative and Alternative
Communication) system written in **C99**.  It lets users type custom messages
or tap a quick-phrase board, then speaks them aloud instantly using the
**built-in Windows speech engine (SAPI)** — no internet, no extra installs.

Voice settings (pitch in semitones, speed, volume) are saved automatically
between sessions, and speech can be exported to WAV files at any time.

---

## Quick Start

1. Open the **OpenVoice** folder.
2. Double-click **`OpenVoice.bat`**.
3. The app opens — use arrow keys to navigate, `Enter` to select.

> **No installation required.**  The app ships as a single `.exe` and uses
> Windows PowerShell (built into Windows 10/11) for voice synthesis.

---

## System Requirements

| Requirement | Version |
|---|---|
| Windows | 10 or 11 (any edition) |
| PowerShell | 5.1 (pre-installed on Windows 10/11) |
| Audio | Any working speaker or headphone output |

Nothing else needs to be installed.

---

## Features

| Feature | Description |
|---|---|
| **Type Message** | Type any text and hear it spoken immediately |
| **Quick Phrase Board** | 4 × 3 grid of common AAC phrases, navigate with arrow keys |
| **Voice Settings** | Adjust pitch (−10 to +10 semitones), speed (80–450 wpm), volume |
| **Export to WAV** | Save synthesised speech as a timestamped PCM WAV file |
| **Prosody Control** | SSML `<prosody>` gives real pitch (F0) and rate modification |
| **Persistent Profile** | Settings saved to `data/profile.cfg` automatically on exit |
| **Works offline** | No internet connection required — all synthesis is local |

---

## Keyboard Reference

| Key | Action |
|---|---|
| `↑` / `↓` | Move menu selection |
| `←` / `→` | Move between phrase buttons / adjust sliders |
| `Enter` | Confirm / speak selected item |
| `1` – `6` | Jump to main-menu item directly |
| `Esc` | Cancel / go back / quit from main menu |
| `S` | Save voice settings |
| `Q` | Quit |

---

## Voice Settings

Open **Voice Settings** (option 3) to adjust:

| Parameter | Range | Default | Notes |
|---|---|---|---|
| **Pitch** | −10 to +10 st | 0 | Semitones above/below the voice's natural F0 |
| **Speed** | 80 – 450 wpm | 175 | Words per minute |
| **Volume** | 0 – 200 | 100 | 100 = system default level |

Press **`S`** to save, **`Esc`** to discard changes.

---

## Customising Phrases

Edit **`data/phrases.txt`** in any text editor.  One phrase per line.
Lines starting with `#` are comments and are ignored.

```
# My custom phrases
Good morning
I need a break
Can you repeat that?
```

The app loads the file on every launch, so changes take effect immediately.
If the file is missing or empty, 20 built-in fallback phrases are used.

---

## WAV Export

Select **Export WAV** (option 4), type your text, and a WAV file is saved
in the OpenVoice folder with a timestamp filename:

```
openvoice_20260614_160300.wav
```

WAV files are standard 16-bit PCM and can be opened in any audio editor.

---

## Project Structure

```
OpenVoice/
├── openvoice.exe        Main application (run this)
├── OpenVoice.bat        Double-click launcher (sets working directory)
├── data/
│   ├── phrases.txt      Quick-phrase board content (edit to customise)
│   └── profile.cfg      Voice settings (saved automatically)
├── src/                 C99 source code
├── include/             Header files
├── tests/               Unit tests (89 tests, run test_*.exe)
├── docs/                Architecture, user guide, report
├── Makefile             Build with: gcc (MinGW on Windows)
└── LICENSE              MIT
```

---

## Building from Source

Requires **GCC** (MinGW recommended on Windows).

```bat
gcc -std=c99 -Wall -I include src\main.c src\ui.c src\speech.c ^
    src\phrase.c src\profile.c src\export.c -o openvoice.exe
```

Or with Make (if installed):

```bat
make
```

### Running Tests

```bat
gcc -std=c99 -I include tests\test_speech.c  src\speech.c  src\profile.c -o tests\test_speech.exe
gcc -std=c99 -I include tests\test_phrase.c  src\phrase.c                 -o tests\test_phrase.exe
gcc -std=c99 -I include tests\test_profile.c src\profile.c                -o tests\test_profile.exe

tests\test_speech.exe
tests\test_phrase.exe
tests\test_profile.exe
```

All 89 tests pass.  The speech tests play audible output — that is intentional.

---

## How Speech Works

OpenVoice uses **Windows SAPI** via a PowerShell one-liner.
No separate TTS engine needs to be installed.

Speech is wrapped in **SSML** (`<prosody>`) so pitch and rate are real
acoustic modifications — not just a label change:

```xml
<speak version="1.0" xmlns="http://www.w3.org/2001/10/synthesis"
       xml:lang="en-us">
  <prosody pitch="+5st" rate="1.20">Hello, I need help.</prosody>
</speak>
```

The live-speech pipeline:
```
SSML → SAPI (SpeakSsml) → temp WAV → SoundPlayer.PlaySync() → delete temp
```

WAV export skips the playback step and writes directly to the named file.

---

## Troubleshooting

| Symptom | Fix |
|---|---|
| **No sound** | Check Windows volume is not muted; run from `OpenVoice.bat` not a network drive |
| **Phrases not loading** | Run from the OpenVoice folder (not a shortcut pointing elsewhere) |
| **Profile not saving** | Make sure the `data/` folder exists next to `openvoice.exe` |
| **"PowerShell not found"** | Install Windows PowerShell 5.1 (included with all Windows 10/11 installs) |

---

## Future Work

- Voice banking — record and clone a user's own voice before it is lost
- Predictive text — suggest completions based on usage history
- Symbol / picture board — AAC symbol grid alongside text
- Switch access — single-switch scanning for users with limited motor control
- Multilingual phrase sets

---

## License

MIT License.  See [LICENSE](LICENSE) for full text.
