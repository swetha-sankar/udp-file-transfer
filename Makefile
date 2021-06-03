all: client server

client: udpcli.c
	gcc -o client udpcli.c

server: udpserv.c
	gcc -o server udpserv.c -lm
