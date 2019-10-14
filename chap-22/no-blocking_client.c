#include<stdio.h>
#include<sys/socket.h>
#include<error.h>
#include<errno.h>
#include<netinet/in.h>
#include<string.h>

#define SERV_PORT 43211

int main(int argc,char **argv){
    if(argc!=2){
        error(1,errno,"usage:nonblockingclieng <IPaddress>");
    }

    int socket_fd=socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in server_addr;
    __bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    socklen_t server_len=sizeof(server_addr);
    if(connect(socket_fd,(struct sockaddr *)&server_addr,server_len)){
        error(1,errno,"connect failed");
    }

    struct linger ling;
    ling.l_onoff = 1;
    ling.l_linger = 0;
    setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
    close(socket_fd);

    exit(0);
}