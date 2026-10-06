# ================================================================
#  Makefile  —  C-IntelliGuard Build System
#  Targets: Windows (MinGW/gcc) and Linux/macOS (gcc)
# ================================================================

CC      = gcc
TARGET  = intelliguard
SRCDIR  = src
BINDIR  = .

# Source files — use the _ops variants for stack/queue
SRCS = $(SRCDIR)/main.c        \
       $(SRCDIR)/file_analyzer.c \
       $(SRCDIR)/statistics.c  \
       $(SRCDIR)/anomaly.c     \
       $(SRCDIR)/stack_ops.c   \
       $(SRCDIR)/queue_ops.c   \
       $(SRCDIR)/kremoval.c    \
       $(SRCDIR)/report.c

OBJS    = $(SRCS:.c=.o)

# Flags
CFLAGS  = -Wall -Wextra -std=c11 -I$(SRCDIR)
LDFLAGS = -lm

# ----------------------------------------------------------------
# Default target
# ----------------------------------------------------------------
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(BINDIR)/$(TARGET) $(LDFLAGS)
	@echo ""
	@echo "  [BUILD OK]  ->  ./$(TARGET)"

# Pattern rule for .c -> .o
$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ----------------------------------------------------------------
# Convenience targets
# ----------------------------------------------------------------
run: all
	./$(TARGET)

clean:
	del /Q $(SRCDIR)\*.o 2>nul || rm -f $(SRCDIR)/*.o
	del /Q $(TARGET).exe 2>nul || rm -f $(TARGET)

# Windows build (explicit)
windows: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET).exe $(LDFLAGS)
	@echo ""
	@echo "  [BUILD OK]  ->  $(TARGET).exe"

# Rebuild from scratch
rebuild: clean all

.PHONY: all run clean windows rebuild
