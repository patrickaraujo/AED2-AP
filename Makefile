# =====================================================================
# Makefile — Árvore Patrícia (variante clássica de Sedgewick)
# =====================================================================
# Uso:
#   make            # compila o executável 'patricia' (5 bits, default)
#   make run        # compila e executa
#   make bits8      # compila com PATRICIA_BITS=8 (chaves de um byte)
#   make bits32     # compila com PATRICIA_BITS=32 (chaves de int)
#   make debug      # build com símbolos para gdb (-g -O0)
#   make sanitize   # build com AddressSanitizer + UBSanitizer
#   make clean      # remove binários e objetos
# =====================================================================

CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -std=c11 -O2
LDFLAGS :=

TARGET  := patricia
OBJS    := main.o patricia.o
HEADERS := patricia.h

# --- Regras principais ------------------------------------------------

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# --- Alvos auxiliares -------------------------------------------------

run: $(TARGET)
	./$(TARGET)

bits8: CFLAGS += -DPATRICIA_BITS=8
bits8: clean $(TARGET)

bits32: CFLAGS += -DPATRICIA_BITS=32
bits32: clean $(TARGET)

debug: CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -g -O0
debug: clean $(TARGET)

sanitize: CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -g -O0 \
                    -fsanitize=address -fsanitize=undefined
sanitize: clean $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all run bits8 bits32 debug sanitize clean
