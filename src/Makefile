CC=gcc
CFLAGS=-O2 -Wall -g
OBJECTS=../lib/objects/mekoy.o ../lib/objects/lucretia.o ../lib/objects/dispatch.o ../lib/objects/process_handler.o ../lib/objects/process_listener.o ../lib/objects/process_message.o ../lib/objects/lcp.o ../lib/objects/util.o ../lib/objects/map.o

.PHONY: clean

mekoy.o: include/mekoy.h mekoy.c
	$(CC) $(CFLAGS) -c mekoy.c
	mkdir -p ../lib/objects &> /dev/null
	mv mekoy.o ../lib/objects/

lucretia.o: dispatch.o lcp.o util.o include/lucretia.h lucretia.c
	$(CC) $(CFLAGS) -c lucretia.c
	mkdir -p ../lib/objects &> /dev/null
	mv lucretia.o ../lib/objects/

dispatch.o: dispatch.c include/dispatch.h
	$(CC) $(CFLAGS) -c dispatch.c
	mkdir -p ../lib/objects &> /dev/null
	mv dispatch.o ../lib/objects/

process_handler.o: process_handler.c include/process_handler.h
	$(CC) $(CFLAGS) -c process_handler.c
	mkdir -p ../lib/objects &> /dev/null
	mv process_handler.o ../lib/objects/

process_listener.o: process_listener.c include/process_listener.h
	$(CC) $(CFLAGS) -c process_listener.c
	mkdir -p ../lib/objects &> /dev/null
	mv process_listener.o ../lib/objects/

process_message.o: process_message.c include/process_message.h
	$(CC) $(CFLAGS) -c process_message.c
	mkdir -p ../lib/objects &> /dev/null
	mv process_message.o ../lib/objects/

lcp.o: lcp.c include/lcp.h
	$(CC) $(CFLAGS) -c lcp.c
	mkdir -p ../lib/objects &> /dev/null
	mv lcp.o ../lib/objects/

util.o: util.c include/util.h
	$(CC) $(CFLAGS) -c util.c
	mkdir -p ../lib/objects &> /dev/null
	mv util.o ../lib/objects/

clean:
	echo "Cleaning object files..."
	rm -rf *.o
	echo "Done."
	echo "Cleaning output files..."
	cd build/
	rm -rf ${OBJECTS}z	