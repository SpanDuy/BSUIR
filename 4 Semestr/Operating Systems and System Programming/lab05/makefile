#makefile
CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic -pthread
.PHONY : clean

all : main

main: main.c makefile
	$(CC) $(CFLAGS) main.c -o main

clean:
	rm -f main