#include<stdio.h>
#include<error.h>
#include<errno.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/select.h>

#define MAXLINE 4096

int main(int argc,char **argv){
    //1. 设置接收参数，接收服务端的ip地址信息和端口号
    if(argc!=3){
        error(1,errno,"usage: tcp_client <IPaddress> <port>");
    }
    char *server_ip=argv[1];
    int server_port=atoi(argv[2]);
    //调用socket函数，创建套接字描述符
    int sockfd;
    sockfd=socket(AF_INET,SOCK_STREAM,0);

    //3. 初始化套接字地址信息
    struct sockaddr_in serveraddr;
    bzero(&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(server_port);
    inet_pton(AF_INET,server_ip,&serveraddr.sin_addr);

    //4. 调用connect函数，与服务器建立联系
    int connect_rt=connect(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
    if(connect_rt<0){
        error(1,errno,"connect failed");
    }

    char send_line[MAXLINE],recv_line[MAXLINE+1];
    int n;

    fd_set readmask;
    fd_set allreads;

    FD_ZERO(&allreads);
    FD_SET(0,&allreads);
    FD_SET(sockfd,&allreads);

    for(;;){
        readmask=allreads;
        int rc=select(sockfd+1,&readmask,NULL,NULL,NULL);
        if(rc<=0){
            error(1,errno,"select failed");
        }
        //当连接套接字上有数据可读，将数据读入到程序缓冲区
        if(FD_ISSET(sockfd,&readmask)){
            n = read(sockfd, recv_line, MAXLINE);
            if (n < 0) {
                error(1, errno, "read error");
            } else if (n == 0) {
                error(1, 0, "server terminated \n");
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }
        //当标准输入上有数据可读时，读入后进行判断
        if(FD_ISSET(0,&readmask)){
            if (fgets(send_line, MAXLINE, stdin) != NULL) {
                if (strncmp(send_line, "quit", 4) == 0) {//比较两个字符串的前8位，相等为0
                    if (shutdown(sockfd, 1)) {
                        error(1, errno, "shutdown failed");
                    }
                } else {
                    int i = strlen(send_line);
                    if (send_line[i - 1] == '\n') {
                        send_line[i - 1] = 0;
                    }

                    size_t rt = write(sockfd, send_line, strlen(send_line));
                    if (rt < 0) {
                        error(1, errno, "write failed ");
                    }
                }
            }
        }
    }

}