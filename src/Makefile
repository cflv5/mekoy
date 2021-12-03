CC=gcc
CFLAGS=-O2 -Wall -g
OBJECTS=

.PHONY: clean

lcp.o: lcp.c include/lcp.h
	$(CC) $(CFLAGS) -c lcp.c
	mv lcp.o ../lib/objects

clean:
	echo "Cleaning object files..."
	rm -rf *.o
	echo "Done."
	echo "Cleaning output files..."
	cd build/
	rm -rf ${OBJECTS}z	