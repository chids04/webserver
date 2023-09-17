#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define PORT "3490" // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once

// Function to get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Function to establish a connection to the server
int connect_to_server(const char *hostname) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // Loop through all the results and connect to the first one
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return -1;
    }

    freeaddrinfo(servinfo); // All done with this structure
    return sockfd;
}

// Function to receive data from the server
int receive_data(int sockfd, char *buf, int max_size) {
    int numbytes;
    if ((numbytes = recv(sockfd, buf, max_size - 1, 0)) == -1) {
        perror("recv");
        return -1;
    }
    buf[numbytes] = '\0';
    return numbytes;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    char buf[MAXDATASIZE];
    int sockfd = connect_to_server(argv[1]);
    if (sockfd == -1) {
        exit(2);
    }

    printf("client: connected to server\n");

    int numbytes = receive_data(sockfd, buf, MAXDATASIZE);
    if (numbytes == -1) {
        exit(1);
    }

    printf("client: received '%s'\n", buf);

    close(sockfd);

    return 0;
}
