# =====================================================================
# Makefile — Árvore B
# =====================================================================
# Uso:
#   make           # compila o executável 'arvoreb'
#   make run       # compila e executa
#   make debug     # compila com -g -O0 (símbolos para gdb)
#   make clean     # remove binários e objetos
# =====================================================================

CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -std=c11 -O2
LDFLAGS :=

TARGET  := arvoreb
OBJS    := main.o TAB.o
HEADERS := TAB.h

# --- Regras principais ------------------------------------------------

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# --- Alvos auxiliares -------------------------------------------------

run: $(TARGET)
	./$(TARGET)

debug: CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -g -O0
debug: clean $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all run debug clean
