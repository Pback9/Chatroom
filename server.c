#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>

// client struct
struct client_t {
	int socketnum;
	char ip[INET_ADDRSTRLEN];
};

int clientSocks[255];
int clientCount = 0;


// message Forwarder
void forwardMessage (char *message, int currentClient) {
	int i;
	// send a message to every client accept the one that sent the message
	for (i = 0; i < clientCount; i++) {
		if (clientSocks[i] != currentClient) {
			if (send(clientSocks[i], message, strlen(message), 0) < 0) {
				perror("Failed sending to client");
				continue;
			}
		}
	}
}

/* This function is to handle all communication with the client*/
void *handle_client(void *arg)
{
    /* Write the business logic you need.*/
	struct  client_t currClient = *((struct client_t *) arg);
	char messageBuff[2048];
	int msgLen;
	int i;
	int j;

	// send a welcome message to the client
	write(currClient.socketnum, "Server: Welcome!\n", strlen("Server: Welcome!\n"));

	// while the client is connected, accept messages and forward them
	while ((msgLen = recv(currClient.socketnum, messageBuff, 2048, 0)) > 0) {
		messageBuff[msgLen] = '\0';
		forwardMessage(messageBuff, currClient.socketnum);
		memset(messageBuff, '\0', sizeof(messageBuff));
	}

	// when the client disconnects, print a message in the server terminal
	// and remove the client from the array of client sockets
	printf("%s has disconnected\n", currClient.ip);
	for (i = 0; i < clientCount; i++) {
		if (clientSocks[i] == currClient.socketnum) {
			j = i;
			while (j < clientCount-1) {
				clientSocks[j] = clientSocks[j+1];
				j++;
			}
		}
	}
	clientCount--;

}

int main(int argc, char *argv[])
{

    int sockfd = 0;
	int clientSockfd = 0;
	socklen_t cli_addr_size;
	char ip[INET_ADDRSTRLEN];

	// check if command lien arguments have been provided correctly
    if (argc != 2)
    {
        printf("Usage: ./server [Port]\n");
        return 0;
    }


	struct sockaddr_in serv_addr, cli_addr;
	pthread_t td;
	struct client_t clientInfo;

	/* Socket settings */
	// create an IPv4 TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd<0) {
        perror("Could not create socket\n");
        return 0;
    }

    printf("Socket created successfully\n");

	// store information about the server network
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = INADDR_ANY;
	cli_addr_size = sizeof(cli_addr);

	/* Bind */
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("Binding failure:");
		return 0;
	}

	/* Listen */
	if ( listen(sockfd, 100) < 0) {
		perror("Listen failure:");
		return 0;
	}

	/* Accept clients */
	while(1){

		/* Client settings */
		if ((clientSockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_addr_size )) < 0) {
			perror("Could not accept client");
			return 0;
		}

		// convert connected ip address from binary to string and print the connected address
		inet_ntop(AF_INET, (struct sockaddr *)&cli_addr, ip, INET_ADDRSTRLEN);
		printf("%s has connected to the server\n", ip);
		// store the ip of the connected client
		clientInfo.socketnum = clientSockfd;
		strcpy(clientInfo.ip, ip);
		clientSocks[clientCount] = clientInfo.socketnum;
		clientCount++;

        /*Create a thread */
		pthread_create(&td, NULL, &handle_client, &clientInfo);

	}

	return 0;
}
