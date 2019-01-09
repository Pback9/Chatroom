#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

// recieve message thread function
void *recieveMessage(void *socketnum) {
    int recSocket = *((int *)socketnum);
    char messageBuff[2048];
    int len;

    // wait for messages and output them whenever they are received
    while ((len = recv(recSocket, messageBuff, 2048, 0)) > 0) {
        messageBuff[len] = '\0';
        fputs(messageBuff, stdout);
        memset(messageBuff, '\0', sizeof(messageBuff));
    }
}

int main(int argc, char *argv[])
{
    int sockfd = 0;
    char messageBuff[1792];
    char recieveBuff[2048];
    pthread_t th;

    // check if command line arguments are provided
    if (argc != 4)
    {
        printf("Usage: ./client [IP Address] [Port] [nickname]\n");
        return 0;
    }

    /* Socket settings */
    // create an IPv4 TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd<0) {
        perror("Could not create socket");
        return 0;
    }

    // set up socket information for the client
    struct sockaddr_in client;

    client.sin_family = AF_INET;
    client.sin_port = htons(atoi(argv[2]));
    client.sin_addr.s_addr = inet_addr(argv[1]);

    /* Request for connecting to the server */

    int conn = connect(sockfd, (struct sockaddr *)&client, sizeof(client));

    if (conn < 0) 
        {
            perror("Could not connect to server");
            return 0;
        }

    printf("Welcome to the chat room!\n");
    /* Send and receive messages */

    // create a separate thread for recieving messages
    pthread_create(&th, NULL, recieveMessage, &sockfd);

    // send message loop
    while (1) {
        fgets(messageBuff, 1792, stdin);
        // if the user enters "exit" the program stops
        if (strcmp(messageBuff, "exit\n") == 0) {
            return 0;
        }

        // concatenate the message to the nickname of the user in the following form:
        // nickname: [message]
        char fullMessage[2048];
        strcpy(fullMessage, argv[3]);
        strcat(fullMessage, ": ");
        strcat(fullMessage, messageBuff);

        // send the message to teh server
        conn = write(sockfd, fullMessage, strlen(fullMessage));

        if (conn < 0) {
            perror("Could not send to server");
            return 0;
        }

        // reset arrays
        memset(fullMessage, '\0', sizeof(fullMessage));
        memset(messageBuff, '\0', sizeof(messageBuff));
    }

    close (sockfd);
    return 0;
}
