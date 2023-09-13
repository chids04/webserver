#include <stdio.h>
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
#define PORT_NUM "6969"

void sigchld_handler(int s){
    
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;

}

void *getAddr(struct sockaddr *sa){

    //only working with ipv4 right now, might add ipv6 support in future
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
}

int main() {
    struct addrinfo hints, *res, *p; 
    struct sockaddr_storage connInfo; //connections address info
    socklen_t connSize;
    struct sigaction sa;
    int yes=1;
    int sockfd, newfd; //sockfd is where listening happens, newfd is the connection itself
    int sr;

    memset(&hints, 0, sizeof(hints)); //makes sure hints struct is cleared
    hints.ai_family = AF_INET; //ipv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; //uses my ip

    if((sr = getaddrinfo(NULL, PORT_NUM, &hints, &res)) != 0){ //fills out structs in linked list
        fprintf(stderr, "address info: %s\n", gai_strerror(sr));
        return 1;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, BACKLOG);

    
    connSize = sizeof(connInfo);
    newfd = accept(sockfd, (struct sockaddr *)&connInfo, &connSize);

}