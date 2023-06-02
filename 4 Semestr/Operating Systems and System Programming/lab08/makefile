#makefile
CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic -pthread -Wno-implicit-function-declaration -Wno-unused-but-set-variable
.PHONY : clean

all : server client

server: server.c makefile
	$(CC) $(CFLAGS) server.c -o server

client: client.c makefile
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm -f server client