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
	cp -t Assignment3-CS17B006-CS17B046 proxy.c README Makefile proxy_parse.c proxy_parse.h 
	tar -cvzf Assignment3-CS17B006-CS17B046.tgz Assignment3-CS17B006-CS17B046/*
