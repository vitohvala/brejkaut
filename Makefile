CC=cc
CFLAGS=-Wall -g -Wextra -std=c99 -pedantic -Wno-deprecated-declarations 
LIBS=`sdl2-config --cflags --libs` -lSDL2_ttf -lm
SRC=src/main.c
BIN=breakout
BINDIR=/usr/local/bin/
all: $(SRC)
	$(CC) $(SRC) -o $(BIN) $(CFLAGS) $(LIBS)
run: all
	./$(BIN)

clean:
	rm -f $(BIN)
uninstall:
	rm -f $(BINDIR)/$(BIN)
