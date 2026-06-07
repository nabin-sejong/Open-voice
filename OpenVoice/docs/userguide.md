# OpenVoice — User Guide

## Introduction

OpenVoice is an AAC (Augmentative and Alternative Communication) tool that
runs in a text terminal.  It lets you — or someone who supports you — type
a message or choose a ready-made phrase, and then hear it spoken aloud
through your computer's speakers.

You do not need a mouse.  Everything is controlled with the keyboard.

---

## Starting OpenVoice

Open a terminal and navigate to the OpenVoice folder, then type:

```
./openvoice
```

The main menu will appear.

---

## Main Menu

```
================================
   OPENVOICE AAC COMMUNICATOR
================================

  1.  Type Message
  2.  Quick Phrases
  3.  Voice Settings
  4.  Export WAV
  5.  About
  6.  Exit
```

**How to navigate:**

| Key | What it does |
|---|---|
| `↑` Arrow Up | Move the highlight up |
| `↓` Arrow Down | Move the highlight down |
| `Enter` | Choose the highlighted option |
| `1`–`6` | Jump directly to that option |

---

## 1 — Type Message

Choose **Type Message** to speak anything you like.

1. A text box will appear.
2. Type your message.
3. Press **Enter** — the message will be spoken aloud.
4. Press **Escape** to cancel without speaking.

**Editing tips while typing:**

| Key | Action |
|---|---|
| `Backspace` | Delete the character to the left |
| `Delete` | Delete the character under the cursor |
| `←` / `→` | Move the cursor left or right |
| `Home` | Jump to the beginning |
| `End` | Jump to the end |

---

## 2 — Quick Phrases

Choose **Quick Phrases** to see a board of ready-made phrases.

- Use the **arrow keys** to move between phrases.
- Press **Enter** to speak the highlighted phrase.
- Press **Escape** to go back without speaking.

The phrases are loaded from the file `data/phrases.txt`.  You (or a
supporter) can open that file in any text editor to add, remove, or
change phrases.

---

## 3 — Voice Settings

Choose **Voice Settings** to adjust how speech sounds.

Three sliders are shown:

| Setting | Range | What it changes |
|---|---|---|
| **Pitch** | 0–99 | How high or low the voice sounds |
| **Speed** | 80–450 | How fast the words are spoken (words per minute) |
| **Volume** | 0–200 | How loud the voice is |

**How to use the sliders:**

| Key | Action |
|---|---|
| `↑` / `↓` | Move between pitch, speed, and volume |
| `←` | Decrease the value |
| `→` | Increase the value |
| `S` | Save the settings and go back |
| `Escape` | Go back without saving |

Settings are automatically saved when you exit the application.

---

## 4 — Export WAV

Choose **Export WAV** to save speech as an audio file you can share or
play back later.

1. Type the text you want to save.
2. Press **Enter**.
3. The audio is saved as **`output.wav`** in the current folder.

The WAV file is a standard PCM audio file and can be played in any
media player (VLC, Windows Media Player, etc.).

---

## 5 — About

Displays information about OpenVoice and its version.

---

## 6 — Exit

Closes OpenVoice.  Your voice settings are saved automatically.

---

## Customising Your Phrase Board

Open `data/phrases.txt` in any text editor.  Each line is one phrase.
Lines that start with `#` are comments and are ignored.

Example:
```
# My custom phrases
Good morning, everyone
I would like a coffee please
Can we turn on the television?
```

Phrases longer than 127 characters will be automatically shortened.

---

## Troubleshooting

**No sound when a phrase is spoken**
- Make sure eSpeak NG is installed: `espeak-ng --version`
- Make sure your speakers are not muted.
- Try increasing the Volume in Voice Settings.

**"WAV export failed" message**
- Confirm eSpeak NG is installed and on your PATH.
- Check that you have write permission to the current directory.

**The screen looks scrambled**
- Make your terminal window larger (at least 80 × 24 characters).
- Try a different terminal emulator that supports colour.

---

## Accessibility Notes

- OpenVoice is designed to be fully usable without a mouse.
- The high-contrast colour scheme (bright text on dark blue) is chosen to
  be readable under most screen conditions.
- If you use a switch access device that emulates keyboard keys, the
  arrow-key navigation should work without modification.
