#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int openSocket(char*, int);
int createIPv4Socket();
struct sockaddr_in* createIPv4Address(char* ip, int port);
int connectToServer(int socketFD, struct sockaddr_in* address);

int main() {
    int port = 2000;
    char* ip = "127.0.0.1"; // Local IP

    const int socketFD = createIPv4Socket();
    if (socketFD == -1) {
        perror("[-]error while opening socket");
        return -1;
    }

    struct sockaddr_in* address = createIPv4Address(ip, port);
    if ( address == NULL) {
        perror("[-] error while creating IPv4 Address");

        return -1;
    };

    int connectResult = connectToServer(socketFD, address);
    if (connectResult == -1) {
        perror("[-] error while connecting to server\n");

        return 0;
    }

    printf("Connected to server\n");

    char* line = NULL;
    size_t lineSize = 0;
    while (true) {
        printf("escriba su expresión ('exit' para salir): ");
        ssize_t charCount = getline(&line, &lineSize, stdin);
        if (charCount == -1) {
            perror("[-] error while reading from stdin - closing socket\n");
            break;
        }

        if (strcmp(line, "exit\n") == 0) {
            printf("exiting...");
            break;
        }

        ssize_t charCountSent = send(socketFD, line, lineSize, 0);
        if (charCountSent == -1) {
            perror("[-] error while sending message - retry please\n");
            break;
        }

        char buffer[1024];
        ssize_t charCountReceived = recv(socketFD, buffer, sizeof(buffer), 0);
        if (charCountReceived == -1) {
            perror("[-]error while receiving response - closing socket\n");
            break;
        }

        printf("[[server]]%s\n", buffer);
    }

    close(socketFD);

    return 0;
}
int createIPv4Socket() {
    /*
     * AF_INET = IPv4
     * SOCK_STREAM = TCP Protocol
     * 0 = Choose protocol based on two previous params
     */
    return socket(AF_INET, SOCK_STREAM, 0);
}

struct sockaddr_in* createIPv4Address(char *ip, int port) {
    struct sockaddr_in* address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port); // convert port number to big endian

    int ptonResult = inet_pton(AF_INET, ip, &address->sin_addr.s_addr); // presentation to network (parse ip string to int32 ip)
    if (ptonResult != 1) {
        perror("[-] error presenting to network");
        free(address);

        return NULL;
    }

    return address;
}

int connectToServer(int socketFD, struct sockaddr_in* address) {
    const int connectResult = connect(socketFD, (struct sockaddr*)address, sizeof(*address));
    free(address);

    return connectResult;
}