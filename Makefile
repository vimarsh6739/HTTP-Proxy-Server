CC=gcc
CFLAGS= -g -Wall

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
	tar -cvzf cos461_ass1_$(USER).tgz proxy.c README Makefile proxy_parse.c proxy_parse.h
