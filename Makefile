CC=gcc
CFLAGS=-Wall -O3 -std=c11
LIBS=-lLZ4

main: main.c
	$(CC) -o main $(CFLAGS) main.c $(LIBS)
