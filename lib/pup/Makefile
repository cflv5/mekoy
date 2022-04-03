CC=gcc
CFLAGS=-O2 -Wall -g

pup_req.o: pup_req.h pup_req.c
	$(CC) $(CFLAGS) -c pup_req.c
	mkdir -p ../objects &> /dev/null
	mv pup_req.o ../objects/

pup_res.o: pup_res.h pup_res.c
	$(CC) $(CFLAGS) -c pup_res.c
	mkdir -p ../objects &> /dev/null
	mv pup_res.o ../objects/