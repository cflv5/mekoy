CC=gcc
CFLAGS=-O2 -Wall -g
OBJECTS=

.PHONY: all

all: list.c list.h
	$(CC) $(CFLAGS) -c list.c
	mkdir -p ../objects/
	mv list.o ../objects/