CC=gcc
CFLAGS=-Wall
all: client server
client: skeleton_client.c
	gcc -pthread -o client skeleton_client.c

server: skeleton_server.c
	gcc -pthread -o server skeleton_server.c
