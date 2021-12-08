CC=gcc
CFLAGS=-O2 -Wall 
OBJECTS=../lib/objects/lcp.test.o ../lib/objects/lucretia.test.o ../lib/objects/lcp.o ../lib/objects/map.o ../lib/objects/util.o ../lib/objects/dispatch.o ../lib/objects/lucretia.o
OUTPUT=test

.PHONY: clean test

test: lcp.test.o lucretia.test.o
	$(CC) $(CFLAGS) $(OBJECTS) main.c -o bin/$(OUTPUT) -luuid

lcp.test.o: lcp.test.c include/lcp.test.h 
	$(CC) $(CFLAGS) -c lcp.test.c
	mv lcp.test.o ../lib/objects

lucretia.test.o: lucretia.test.c include/lucretia.test.h
	$(CC) $(CFLAGS) -c lucretia.test.c
	mv lucretia.test.o ../lib/objects

clean:
	echo "Cleaning object files..."
	rm -rf *.o
	echo "Done."
	echo "Cleaning output files..."