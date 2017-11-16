src = $(wildcard *.c)
obj = $(src:.c=.o)
CC=gcc
CFLAGS = -Wall -s -O3 -I/usr/include/freetype2
LDFLAGS = -lGL -lGLEW -lfreetype -lSDL2 -lm

%.c:
	$(CC) $(CFLAGS) -o $@

pong3d: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) pong3d

