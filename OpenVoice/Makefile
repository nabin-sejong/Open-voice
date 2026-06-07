# ===========================================================================
# OpenVoice AAC Communicator - Makefile
# ===========================================================================
# Usage:
#   make            Build the main application
#   make tests      Build and run all unit tests
#   make clean      Remove all build artefacts
#   make install    Install to /usr/local/bin (requires root)
#
# Dependencies:
#   gcc, ncurses-dev, espeak-ng
# ===========================================================================

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -g \
          -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lncurses

TARGET  = openvoice

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
	@echo "  Build complete: ./$(TARGET)"
	@echo "  Run with:  ./$(TARGET)"
	@echo ""

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

# ---------------------------------------------------------------------------
# Unit tests
# ---------------------------------------------------------------------------
TEST_SRCDIR = tests
TEST_BINS   = test_profile test_phrase test_speech

.PHONY: tests
tests: $(TEST_BINS)
	@echo ""
	@echo "=== Running Unit Tests ==="
	@./test_profile
	@./test_phrase
	@./test_speech
	@echo "=== All tests complete ==="
	@echo ""

test_profile: $(TEST_SRCDIR)/test_profile.c $(SRCDIR)/profile.c
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $^

test_phrase: $(TEST_SRCDIR)/test_phrase.c $(SRCDIR)/phrase.c
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $^

test_speech: $(TEST_SRCDIR)/test_speech.c $(SRCDIR)/speech.c $(SRCDIR)/profile.c
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $^

# ---------------------------------------------------------------------------
# Install / uninstall
# ---------------------------------------------------------------------------
PREFIX ?= /usr/local

.PHONY: install
install: $(TARGET)
	install -m 755 $(TARGET) $(PREFIX)/bin/$(TARGET)
	@echo "Installed to $(PREFIX)/bin/$(TARGET)"

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)

# ---------------------------------------------------------------------------
# Clean
# ---------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(TARGET) $(TEST_BINS)
	@echo "Cleaned build artefacts."
