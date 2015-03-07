# A wanna-be generic Makefile (C) 2006 vah
TARGET=testcard

# DO NOT MODIFY BELOW THIS LINE (unless you know what you are doing)
.SUFFIXES:
.SUFFIXES: .c .o

.PHONY=all clean distclean

all: $(TARGET)

clean:
	@rm -f *~ *.o */*.o */*~

distclean: clean
	@rm -f $(TARGET)

srcdir:=.
SRC=$(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*/*.c)
OBJ=$(patsubst %.c,%.o,$(SRC))

CC:=gcc
LD:=gcc

CFLAGS += -pipe -W -Wall -std=c99
CFLAGS += -g -Os -finline-functions $(shell sdl-config --cflags)
LIBS = -lm $(shell sdl-config --libs) -lSDL_ttf


$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

