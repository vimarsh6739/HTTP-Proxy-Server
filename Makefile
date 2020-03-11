CC=gcc
CFLAGS= -g -Wall -O3

all: proxy request

proxy: proxy.c 
	$(CC) $(CFLAGS) -o proxy_parse.o -c proxy_parse.c
	$(CC) $(CFLAGS) -o proxy.o -c proxy.c
	$(CC) $(CFLAGS) -o proxy proxy_parse.o proxy.o

request: request_generator.c
	$(CC) $(CFLAGS) -o request request_generator.c

clean:
	rm -f proxy request *.o

tar:
	mkdir -p Assignment3-CS17B006-CS17B046
	cp -r -t Assignment3-CS17B006-CS17B046 *
	tar -cvzf Assignment3-CS17B006-CS17B046.tar.gz Assignment3-CS17B006-CS17B046/*
