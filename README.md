# OpenVoice — AAC Communicator

> **Augmentative and Alternative Communication for users with speech impairments**

OpenVoice is a free, open-source, terminal-based AAC (Augmentative and
Alternative Communication) system written in **C99**.  It lets users type
custom messages or select pre-loaded quick phrases, then speaks them aloud
using [eSpeak NG](https://github.com/espeak-ng/espeak-ng).  Voice settings
(pitch, speed, volume) are saved between sessions, and speech can be exported
to WAV files.

---

## Features

| Feature | Description |
|---|---|
| **Type Message** | Enter any text and hear it spoken immediately |
| **Quick Phrase Board** | Navigate a grid of common AAC phrases with arrow keys |
| **Voice Settings** | Adjust pitch (0–99), speed (80–450 wpm), and volume (0–200) |
| **Export to WAV** | Save synthesised speech as a standard PCM WAV file |
| **Persistent Profile** | Settings are saved to `data/profile.cfg` on exit |
| **High-contrast TUI** | ncurses interface with keyboard navigation |

---

## Requirements

| Dependency | Ubuntu/Debian | Fedora/RHEL | macOS (Homebrew) |
|---|---|---|---|
| GCC (C99) | `gcc` | `gcc` | `gcc` |
| ncurses | `libncurses-dev` | `ncurses-devel` | `ncurses` |
| eSpeak NG | `espeak-ng` | `espeak-ng` | `espeak-ng` |
| CMake (optional) | `cmake` | `cmake` | `cmake` |

---

## Installation

### 1 — Install dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install gcc libncurses-dev espeak-ng make
```

### 2 — Clone the repository

```bash
git clone https://github.com/yourname/OpenVoice.git
cd OpenVoice
```

### 3 — Compile

**Using Make (recommended):**

```bash
make
```

**Using CMake:**

```bash
mkdir build && cd build
cmake ..
make
```

---

## Usage

```bash
./openvoice
```

The application starts with data files relative to the **current working
directory**, so always launch it from the project root.

### Navigation

| Key | Action |
|---|---|
| `↑` / `↓` | Move menu selection |
| `←` / `→` | Adjust slider values (Voice Settings) |
| `Enter` | Confirm / speak selected item |
| `1`–`6` | Jump to menu item directly |
| `Esc` | Cancel / go back |
| `S` | Save voice settings |
| `Q` | Quit |

### Quick-phrase file

Edit `data/phrases.txt` to customise your phrase board.  One phrase per line;
lines beginning with `#` are treated as comments.

---

## Running Tests

```bash
make tests
```

Tests cover: phrase loading, profile persistence, and speech-command
construction — no audio device required.

---

## Project Structure

```
OpenVoice/
├── src/            Application source files (C99)
├── include/        Public header files
├── data/           phrases.txt and profile.cfg
├── docs/           Architecture, UserGuide, and Report
├── tests/          Unit tests
├── screenshots/    Terminal screenshots
├── Makefile        GNU Make build system
└── CMakeLists.txt  CMake build system
```

---

## Screenshots

*(See the `screenshots/` directory for terminal recordings.)*

---

## Future Work

- **Custom speech synthesis** — integrate a neural TTS engine (e.g. Piper)
- **Voice banking** — record and clone a user's own voice before it is lost
- **Predictive text** — suggest completions based on usage history
- **Android / iOS port** — mobile version using platform TTS APIs
- **Multilingual support** — full phrase sets in multiple languages
- **Symbol/picture support** — AAC symbol board in addition to text
- **Switch access** — single-switch scanning for users with limited mobility
