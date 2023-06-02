#makefile
CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic -pthread
.PHONY : clean

all : main_1 main_2

main_1: main_1.c makefile
	$(CC) $(CFLAGS) main_1.c -o main_1

main_2: main_2.c makefile
	$(CC) $(CFLAGS) main_2.c -o main_2

clean:
	rm -f main_1 main_2