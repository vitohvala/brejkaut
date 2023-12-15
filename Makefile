CC=cc
CFLAGS=-Wall -g -Wextra -std=c99 -pedantic -Wno-deprecated-declarations -Os
LIBS=`sdl2-config --cflags --libs` -lm
SRC=src/main.c
BIN=breakout
BINDIR=/usr/local/bin/
all: $(SRC)
	$(CC) $(SRC) -o $(BIN) $(CFLAGS) $(LIBS)
install: $(BIN)
	install -d $(BINDIR)
	install $(BIN) $(BINDIR)
clean:
	rm -f $(BIN)
uninstall:
	rm -f $(BINDIR)/$(BIN)
