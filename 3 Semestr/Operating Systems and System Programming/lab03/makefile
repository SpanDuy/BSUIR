#makefile
CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic
.PHONY : clean

all : parent child

parent: parent.c makefile
	$(CC) $(CFLAGS) parent.c -o parent
	
child: child.c makefile
	 $(CC) $(CFLAGS) child.c -o child

clean:
	rm -f parent child
