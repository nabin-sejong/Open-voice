# ===========================================================================
# OpenVoice AAC Communicator - Makefile  (Windows / MinGW)
# ===========================================================================
# Usage:
#   make            Build openvoice.exe
#   make tests      Build and run all unit tests
#   make clean      Remove all build artefacts
#
# Requirements:
#   MinGW gcc  (https://www.mingw-w64.org/)
#   No external libraries needed — uses Windows Console API + Windows SAPI.
# ===========================================================================

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -g
# Windows Console API is linked automatically by MinGW (kernel32)
LDFLAGS =

TARGET  = openvoice.exe

SRCDIR  = src
INCDIR  = include
OBJDIR  = build

SRCS = $(SRCDIR)/main.c    \
       $(SRCDIR)/ui.c      \
       $(SRCDIR)/speech.c  \
       $(SRCDIR)/phrase.c  \
       $(SRCDIR)/profile.c \
       $(SRCDIR)/export.c

OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

# ---------------------------------------------------------------------------
# Default target: main application
# ---------------------------------------------------------------------------
.PHONY: all
all: $(OBJDIR) $(TARGET)
	@echo ""
	@echo "  Build complete: $(TARGET)"
	@echo "  Run with:  $(TARGET)"
	@echo ""

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

# ---------------------------------------------------------------------------
# Unit tests  (no UI dependency — run without Windows Console setup)
# ---------------------------------------------------------------------------
TEST_SRCDIR = tests
TEST_BINS   = test_profile.exe test_phrase.exe test_speech.exe

.PHONY: tests
tests: $(TEST_BINS)
	@echo ""
	@echo "=== Running Unit Tests ==="
	@./test_profile.exe
	@./test_phrase.exe
	@./test_speech.exe
	@echo "=== All tests complete ==="
	@echo ""

test_profile.exe: $(TEST_SRCDIR)/test_profile.c $(SRCDIR)/profile.c
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $^

test_phrase.exe: $(TEST_SRCDIR)/test_phrase.c $(SRCDIR)/phrase.c
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $^

test_speech.exe: $(TEST_SRCDIR)/test_speech.c $(SRCDIR)/speech.c $(SRCDIR)/profile.c
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $^

# ---------------------------------------------------------------------------
# Clean
# ---------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(TARGET) $(TEST_BINS)
	@echo "Cleaned build artefacts."
