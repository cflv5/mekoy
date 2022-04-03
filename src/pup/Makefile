CC=gcc
CFLAGS=-O2 -Wall -g
OBJECTS=pup_command_handlers.o pup_filters.o ../../lib/objects/list.o ../../lib/objects/pup_req.o ../../lib/objects/pup_res.o

pup: pup_command_handlers.o pup_filters.o pup.c include/pup.h
	$(CC) $(CFLAGS) $(OBJECTS) pup.c -o pup
.PHONY: pup

pup_filters.o: pup_filters.c include/pup_filters.h
	$(CC) $(CFLAGS) -c pup_filters.c

pup_command_handlers.o: pup_command_handlers.c include/pup_command_handlers.h
	$(CC) $(CFLAGS) -c pup_command_handlers.c

clean:
	rm -rf *.o
.PHONY: clean