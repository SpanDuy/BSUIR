#makefile
CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic -pthread
.PHONY : clean

all : main produser consumer

main: main.c makefile
	$(CC) $(CFLAGS) main.c -o main

produser: produser.c makefile
	$(CC) $(CFLAGS) produser.c -o produser

consumer: consumer.c makefile
	$(CC) $(CFLAGS) consumer.c -o consumer

clean:
	rm -f main produser consumer