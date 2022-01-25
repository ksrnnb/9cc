CFLAGS=-std=c11 -g -static

9cc: 9cc.c

test: 9cc
		./test.sh

# https://unix.stackexchange.com/questions/6781/what-does-mean
clean:
		rm -f 9cc *.o *~ tmp*

.PHONY: test clean