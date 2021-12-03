CC=gcc
CFLAGS=-O2 -Wall -g
OBJECTS=

.PHONY: clean

lucretia.o: mappings.o lcp.o util.o include/lucretia.h lucretia.c
	$(CC) $(CFLAGS) -c lucretia.c
	mv lucretia.o ../lib/objects

mappings.o: mappings.c include/mappings.h
	$(CC) $(CFLAGS) -c mappings.c
	mv mappings.o ../lib/objects

lcp.o: lcp.c include/lcp.h
	$(CC) $(CFLAGS) -c lcp.c
	mv lcp.o ../lib/objects

util.o: util.c include/util.h
	$(CC) $(CFLAGS) -c util.c
	mv util.o ../lib/objects

clean:
	echo "Cleaning object files..."
	rm -rf *.o
	echo "Done."
	echo "Cleaning output files..."
	cd build/
	rm -rf ${OBJECTS}z	