CC=g++
CFLAGS= -g -Wall -Werror

all: proxy

proxy: proxy.c
	$(CC) $(CFLAGS) -o proxy.o -c proxy.c
	$(CC) $(CFLAGS) -o proxy proxy.o

clean:
	rm -f proxy *.o

tar:
	tar -cvzf CS3205_Assignment3_$(USER).tar.gz proxy.c README Makefile
