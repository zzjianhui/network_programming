#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<error.h>
#include<errno.h>
#include<wait.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>

#define SERV_PORT 43211
#define LISTENQ 1024
#define MAX_LINE 1024

int tcp_server_listen(int port){
    int listen_fd;
    listen_fd=socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(SERV_PORT);

    if(bind(listen_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
        error(1,errno,"bind failed");
    }

    if(listen(listen_fd,LISTENQ)<0){
        error(1,errno,"bind failed");
    }

    return listen_fd;
}

void sigchld_handler(int sig){
    while(waitpid(-1,0,WNOHANG)>0);
    return ;
}

//将字符c的位置往前移动13位或往后移动13位。
char rot13_char(char c) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

void child_run(int fd){
    char outbuf[MAX_LINE+1];
    size_t outbuf_used=0;
    ssize_t result;

    while(1){
        char ch;
        result=recv(fd,&ch,1,0);
        if(result==0){
            break;
        }else if(result==-1){
            error(1,errno,"read failed");
            break;
        }

        if(outbuf_used<sizeof(outbuf)){
            outbuf[outbuf_used++]=rot13_char(ch);
        }

        if(ch == '\n'){
            send(fd,outbuf,outbuf_used,0);
            outbuf_used=0;
            continue;
        }
    }
}

int main(int argc,char **argv){
    int listen_fd=tcp_server_listen(SERV_PORT);

    signal(SIGCHLD,sigchld_handler);//一旦发生进程terminate或stop，在父进程中调用该函数

    while(1){
        struct sockaddr_storage ss;
        socklen_t slen=sizeof(ss);
        int fd=accept(listen_fd,(struct sockaddr *)&ss,&slen);
        if(fd<0){
            error(1,errno,"accept failed");
            exit(1);
        }

        if(fork()==0){
            close(listen_fd);
            child_run(fd);
            exit(0);
        }else{
            close(listen_fd);
        }
    }

    return 0;
}