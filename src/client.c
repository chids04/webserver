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

#define PORT_NUM "1023"
#define HOSTNAME "cmac.local"
#define MAXBUFFSIZE 300

void *getAddr(struct sockaddr *sa){

    //only working with ipv4 right now, might add ipv6 support in future
    sa->sa_family = AF_INET;
    return &(((struct sockaddr_in*)sa)->sin_addr);
    
}

int conn2serv(){
    int sockfd, res;
    struct addrinfo hints, *servinfo, *p;
    char servip[INET_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if((res = getaddrinfo(HOSTNAME, PORT_NUM, &hints, &servinfo)) != 0){
        fprintf(stderr, "error getting sockets: %s/n", gai_strerror(res));
        return 1;
    }

    for(p=servinfo; p!= NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("could not get socket file descriptor, trying next one");
            continue;
        }

        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("could not connect to this socket, trying next socket");
            continue;
        }
        
        break;
    }

    if(p==NULL){
        perror("could not find any sockets to connect to");
        return 1;
    }

    inet_ntop(p->ai_family, getAddr((struct sockaddr*)p->ai_addr), servip, sizeof(servip));
    printf("client connecting to: %s\n", servip);

    freeaddrinfo(servinfo);

    return sockfd;
}

void handleConn(int sockfd){
    char buff[MAXBUFFSIZE];
    int numbytes;

    if((numbytes = recv(sockfd, buff, MAXBUFFSIZE-1, 0)) == -1){
        perror("error recieving data");
        exit(1);
    }

    buff[numbytes] = '\0';

    printf("%s\n", buff);


}

void sendRequest(int sockfd, char *message){
    int bytes_sent;

    if((bytes_sent = send(sockfd, message, strlen(message), 0)) == -1){
        perror("error sending message");
        exit(1);
    }

    printf("sent: %s\n", message);
}

int main(int argc, char *argv[]) {
    int sockfd = conn2serv();

    if(sockfd == 1){
        perror("error connecting to server");
    }

    handleConn(sockfd);
    sendRequest(sockfd, "hello");

    return 0;
}