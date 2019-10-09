#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<error.h>
#include<errno.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>

#define SERV_PORT 43211
#define LISTENQ 1024
#define MAXLINE 4096

static int count;

static void sig_int(int signo){
    printf("\nrecvived %d datagrams\n",count);
    exit(0);
}

int main(int argc,char **argv){
    //1. 调用socket函数，创建套接字描述符
    int listenfd;
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    //2. 初始化套接字地址信息，(服务端的IP和端口)
    struct sockaddr_in server_addr;
    __bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(SERV_PORT);
    //3. 调用bind函数，将套接字和套接字地址信息进行绑定
    int rt1=bind(listenfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    if(rt1<0){
        error(1,errno,"bind failed");
    }

    //4. 调用listen函数，
    int rt2=listen(listenfd,LISTENQ);
    if(rt2<0){
        error(1,errno,"listen failed");
    }

    //5. 
    signal(SIGINT,sig_int);//这个函数是当告诉系统发生中断的时候该干什么，第一个参数就是信号的编号(这里为中断信号)，第二个参数就是信号的指针(自定义的信号处理函数)。
    signal(SIGPIPE,SIG_IGN);//传入SIG_IGN表示处理方式为忽略信号，内核会直接将信号丢弃，不会传递给进程

    //6. 调用accept函数，返回监听描述符 
    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len=sizeof(client_addr);
    if((connfd=accept(listenfd,(struct sockaddr *)&client_addr,&client_len))<0){
        error(1,errno,"bind failed");
    }

    char message[MAXLINE];
    count=0;

    for(;;){
        //调用read函数接收客户端的数据
        int n=read(connfd,message,MAXLINE);
        if(n<0){
            error(1,errno,"error read");
        }else if(n==0){
            error(1,0,"client closed \n");
        }
        message[n]=0;
        printf("received %d bytes: %s\n",n,message);
        count++;
        //发送数据到客户端上面
        char send_line[MAXLINE];
        sprintf(send_line,"Hi,%s",message);

        sleep(5);

        int write_nc=send(connfd,send_line,strlen(send_line),0);
        printf("send bytes:%zu \n",write_nc);
        if(write_nc<0){
            error(1,errno,"error write");
        }
    }

}