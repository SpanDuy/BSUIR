#makefile
CC = gcc
CFLAGS = -W -Wall -std=c11 -pedantic -pthread -Wno-unused-parameter -Wno-unused-variable
.PHONY : clean

all : main generate show

main: main.c makefile
	$(CC) $(CFLAGS) main.c -o main

generate: generate.c makefile
	$(CC) $(CFLAGS) generate.c -o generate

show: show.c makefile
	$(CC) $(CFLAGS) show.c -o show

clean:
	rm -f main generate show