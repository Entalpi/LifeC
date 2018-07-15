CC = gcc
CFLAGS = -std=c11 -pedantic -Wall 
LIBS = -lncurses
EXENAME = -o 'lifec'

default: main.c
	$(CC) $(CFLAGS) $(LIBS) $(EXENAME) main.c