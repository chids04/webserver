#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 5
#define PORT_NUM "1023"
#define CLIENT_IP "127.0.0.1"
#define MAXBUFFSIZE 300

void sigchld_handler(int s){
    
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;

}

void *getAddr(struct sockaddr *sa){

    //only working with ipv4 right now, might add ipv6 support in future
    sa->sa_family = AF_INET;
    return &(((struct sockaddr_in*)sa)->sin_addr);
    
}

int createServerSocket(){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int yes = 1;
    int sr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // type of socket im using
    hints.ai_flags = AI_PASSIVE; // uses my IP

    if ((sr = getaddrinfo(NULL, PORT_NUM, &hints, &servinfo)) != 0) {
        fprintf(stderr, "address info: %s\n", gai_strerror(sr));
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("error in setting server socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("error in setting socket option");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("error binding socket");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        perror("could not bind to a socket");
        return -1;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("cant listen on binded socket");
        return -1;
    }

    return sockfd;
}

void handleRequest(int newfd){
    int bytes;
    char buff[MAXBUFFSIZE];

    if((bytes = recv(newfd, buff, MAXBUFFSIZE-1, 0)) == -1){
        perror("error recieving data");
        return;
    }

    buff[bytes] = '\0';
    printf("message recieved: %s\n", buff);

}

int handleConn(int sockfd){
    struct sockaddr_storage connInfo;
    socklen_t connSize;
    char clientip[INET_ADDRSTRLEN];
    int newfd;
    int success = 0;
    char *message = "HTTP/1.0 200 OK";


    while(1){
        connSize = sizeof(connInfo);
        newfd = accept(sockfd, (struct sockaddr*)&connInfo, &connSize);
        if (newfd == -1) {
            perror("error accepting socket");
            return -1;
        }

        inet_ntop(connInfo.ss_family, getAddr((struct sockaddr*)&connInfo), clientip, sizeof(clientip));
        printf("got connection from: %s\n", clientip);

        if (!fork()) {
            close(sockfd);

            if (send(newfd, message, (int)strlen(message), 0) == -1) {
                perror("error sending message to client");
                success = -1;
            }

            if(success == 0){
                handleRequest(newfd);
            }
            

            close(newfd);
            exit(0);
        }
        close(newfd);

    }

}

int main() {
    int sockfd = createServerSocket();
    bool running = true;
    if (sockfd == -1) {
        exit(1);
    }

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL)) {
        perror("sigaction");
        exit(1);
    }

    handleConn(sockfd);

    return 0;
}