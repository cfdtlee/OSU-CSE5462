CC= gcc
CFLAGS=-c -Wall

all: ftpc ftps tcpd

ftpc: ftpc.c lib.o
	$(CC) ftpc.c lib.o -o ftpc

ftps: ftps.c lib.o
	$(CC) ftps.c lib.o -o ftps

tcpd: tcpd.c
	$(CC) tcpd.c lib.o -o tcpd
	
lib.o: lib.c
	$(CC) $(CFLAGS) lib.c

clean:
	rm *o ftpc ftps tcpd