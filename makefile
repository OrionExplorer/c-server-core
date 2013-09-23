CC=gcc 
CFLAGS=-std=c99 -g

ifeq ($(OS),Windows_NT)
	LIBS += -lws2_32
endif

all: server

server: socket_io.o server.o
	gcc server.o socket_io.o -o server $(LIBS)

server.o: source/server.c
	gcc -c source/server.c $(CFLAGS)

socket_io.o: source/socket_io.c
	gcc -c source/socket_io.c $(CFLAGS)

clean:
	rm *.o
