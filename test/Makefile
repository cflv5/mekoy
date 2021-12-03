CC=gcc
CFLAGS=-O2 -Wall 
OBJECTS=../lib/objects/lcp.test.o ../lib/objects/lcp.o
OUTPUT=test

.PHONY: clean test

test: lcp.test.o
	$(CC) $(CFLAGS) $(OBJECTS) main.c -o bin/$(OUTPUT)

lcp.test.o: lcp.test.c include/lcp.test.h 
	$(CC) $(CFLAGS) -c lcp.test.c
	mv lcp.test.o ../lib/objects

clean:
	echo "Cleaning object files..."
	rm -rf *.o
	echo "Done."
	echo "Cleaning output files..."