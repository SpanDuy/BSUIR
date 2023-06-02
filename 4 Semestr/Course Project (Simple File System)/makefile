#makefile
CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic -pthread
.PHONY : clean

all : sfs 

sfs: ./src/sfs.c makefile
	$(CC) $(CFLAGS) ./src/sfs.c -o ./src/sfs

clean:
	rm -f sfs 