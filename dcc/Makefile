CFLAGS = -std=c11 -g -static

dcc: dcc.c

test: dcc
	./test.sh

clean:
	rm -f dcc *.o *~ tmp* core

.PHONY: test clean