CC=gcc
CFLAGS=-std=gnu99 -pedantic-errors

all:
	$(CC) lexer.c main.c -o a3shell $(CFLAGS)
