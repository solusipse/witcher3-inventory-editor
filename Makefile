CC=gcc
CFLAGS=-Wall -O3
LIBS=-lLZ4

main: main.c
	$(CC) -o main $(CFLAGS) $(LIBS) main.c
