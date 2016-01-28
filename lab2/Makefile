# Makefile for client and server

CC = gcc
OBJCLI = ftpc.c 
OBJSRV = ftps.c
CFLAGS = 
# setup for system
LIBS =

all: ftpc ftps 

ftpc:	$(OBJCLI)
	$(CC) $(CFLAGS) $(OBJCLI) $(LIBS) -o $@

ftps:	$(OBJSRV)
	$(CC) $(CFLAGS) $(OBJSRV) $(LIBS) -o $@

clean:
	rm ftpc ftps