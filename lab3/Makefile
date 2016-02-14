# Makefile for client and server

CC = gcc
OBJCLI = ftpc.c 
OBJSRV = ftps.c
CFLAGS=-c -Wall
# setup for system
LIBS = 

all: ftpc ftps tcpd function.o

ftpc: ftpc.c function.o
	$(CC) ftpc.c function.o -o ftpc

ftps: ftps.c function.o
	$(CC) ftps.c function.o -o ftps

tcpd: tcpd.c
	$(CC) tcpd.c -o tcpd
	
function.o: function.c
	$(CC) $(CFLAGS) function.c

clean:
	rm *o ftpc ftps tcpd