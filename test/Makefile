CC=gcc
CFLAGS=-O2 -Wall -g
OBJECTS=../lib/objects/lcp.test.o ../lib/objects/lucretia.test.o ../lib/objects/process_message.test.o ../lib/objects/lcp.o ../lib/objects/process_message.o ../lib/objects/map.o ../lib/objects/util.o ../lib/objects/dispatch.o ../lib/objects/lucretia.o
OUTPUT=test

.PHONY: clean test

test: lcp.test.o lucretia.test.o process_message.test.o
	mkdir -p bin/ &> /dev/null
	$(CC) $(CFLAGS) $(OBJECTS) main.c -o bin/$(OUTPUT) -luuid

process_message.test.o: process_message.test.c include/process_message.test.h 
	$(CC) $(CFLAGS) -c process_message.test.c
	mkdir -p ../lib/objects &> /dev/null
	mv process_message.test.o ../lib/objects/

lcp.test.o: lcp.test.c include/lcp.test.h 
	$(CC) $(CFLAGS) -c lcp.test.c
	mkdir -p ../lib/objects &> /dev/null
	mv lcp.test.o ../lib/objects/

lucretia.test.o: lucretia.test.c include/lucretia.test.h
	$(CC) $(CFLAGS) -c lucretia.test.c
	mkdir -p ../lib/objects &> /dev/null
	mv lucretia.test.o ../lib/objects/

clean:
	echo "Cleaning object files..."
	rm -rf *.o
	echo "Done."
	echo "Cleaning output files..."