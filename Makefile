CC=cc
CFLAGS=-Wall -g -Wextra -std=c99 -pedantic -Wno-deprecated-declarations -Waddress -Waggregate-return -Waggressive-loop-optimizations -Warray-bounds -Wbad-function-cast -Wcast-align -Wclobbered -Wdeprecated -Wdeprecated-declarations -Wdesignated-init  -Wdiscarded-array-qualifiers -Wdiscarded-qualifiers -Wdiv-by-zero -Wempty-body -Wendif-labels -Wenum-compare -Wfloat-equal -Wformat-contains-nul -Wimplicit -Wimplicit-function-declaration -Wimplicit-int -Wincompatible-pointer-types -Winit-self -Wint-conversion -Wlogical-not-parentheses -Wmain -Wmaybe-uninitialized -Wmemset-transposed-args -Wmissing-braces -Wmissing-declarations -Wmissing-field-initializers -Wmissing-parameter-type -Wmissing-prototypes -Wmultichar -Wnarrowing -Wnested-externs -Wnonnull -Wodr -Woverflow -Woverlength-strings -Woverride-init -Wparentheses -Wpointer-arith -Wpointer-sign -Wpointer-to-int-cast -Wpragmas  -Wreturn-type -Wsequence-point -Wshadow -Wshift-count-negative -Wshift-count-overflow -Wsign-compare -Wsizeof-array-argument -Wsizeof-pointer-memaccess -Wstack-protector  -Wstrict-prototypes -Wuninitialized -Wunsafe-loop-optimizations -Wunused -Wunused-but-set-parameter -Wunused-but-set-variable -Wunused-function -Wunused-label -Wunused-local-typedefs -Wunused-macros -Wunused-parameter -Wunused-result -Wunused-value -Wunused-variable -Wwrite-strings 
LIBS=-I/usr/local/include/SDL2 -D_REENTRANT -L/usr/local/lib -lSDL2 -lm -ldl -lpthread -lrt
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
