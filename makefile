VPATH=src:include

CFLAGS=-I. -O2 -std=gnu2x -Wall -Wextra -march=cannonlake -mtune=intel
CC=gcc
DEPS=cells.h genetics.h math.h rng.h world.h

cells: main.o cells.o genes.o math.o rng.o world.o
	$(CC) $(CFLAGS) -o cells $^

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o
