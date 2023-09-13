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
    struct addrinfo hints, *servinfo, *p; 
    struct sockaddr_storage connInfo; //connections address info
    socklen_t connSize;
    struct sigaction sa;
    int yes=1;
    int sockfd, newfd; //sockfd is where listening happens, newfd is the connection itself
    int sr;

    memset(&hints, 0, sizeof(hints)); 
    hints.ai_family = AF_INET; //ipv4
    hints.ai_socktype = SOCK_STREAM; //type of socket im using
    hints.ai_flags = AI_PASSIVE; //uses my ip

    if((sr = getaddrinfo(NULL, PORT_NUM, &hints, &servinfo)) != 0){ //fills out structs in linked list
        fprintf(stderr, "address info: %s\n", gai_strerror(sr));
        return 1;
    }

    for(p = servinfo; p!=NULL; p = p->ai_next){ //loops thru results from getaddrinfo and attempts to bind to first compatible one
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("error in setting server socket");
            continue;
        }
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){ //allows reusing port if alr occupied by kernel process
            perror("error in setting socket option");
            continue;
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            perror("error binding socket");
            continue;
        }
    }


    
    connSize = sizeof(connInfo);
    newfd = accept(sockfd, (struct sockaddr *)&connInfo, &connSize);

}