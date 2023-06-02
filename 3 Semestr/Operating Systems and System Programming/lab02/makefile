CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -Wno-unused-parameter
.PHONY: clean

all: parent child

parent: parent.c makefile
		$(CC) $(CFLAGS) parent.c -o parent

child: child.c makefile
		$(CC) $(CFLAGS) child.c -o child

clean:
	 	rm -f parent child