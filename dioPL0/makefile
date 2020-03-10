CC = gcc
PL0 = ./diopl0
CFLAGS = -Wall -Wno-strict-aliasing -std=gnu11 -g -I. -O0

OBJS = codegen.o compile.o getSource.o main.o table.o

.SUFFIXES: .o .c

.c.o:
	$(CC) $(CFLAGS) -c $<

diopl0: ${OBJS}
	$(CC) -o $@ ${OBJS}

clean:
	rm -rf *~ *.o
	rm -rf diopl0
	rm -rf *.html
	rm -rf ./test/*.html

.PHONY: clean