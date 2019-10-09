#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<error.h>
#include<errno.h>
#include<string.h>
#include<sys/select.h>
#include<stdlib.h>
#include<unistd.h>

#define SERV_PORT 43211
#define MAXLINE 14096

int main(int argc,char **argv){
    //1. 接收参数，服务端的ip地址
    if(argc!=2){
        error(1,0,"usage:graceclient <IPaddress>");
    }
    
    //2. 调用socket函数，返回描述符
    int socket_fd;
    socket_fd=socket(AF_INET,SOCK_STREAM,0);
    
    //3. 初始化套接字地址信息，设置端口号和IP地址(为服务端的IP地址和端口)
    struct sockaddr_in server_addr;
    __bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(SERV_PORT);
    inet_pton(AF_INET,argv[1],&server_addr.sin_addr);

    //4. 调用connect函数，与服务端建立连接
    socklen_t server_len=sizeof(server_addr);
    int connetc_rt=connect(socket_fd,(struct sockaddr *)&server_addr,server_len);
    if(connetc_rt<0){
        error(1,errno,"connect failed");
    }

    char send_line[MAXLINE],recv_line[MAXLINE+1];
    int n;

    fd_set readmask;
    fd_set allreads;

    FD_ZERO(&allreads);
    FD_SET(0,&allreads);
    FD_SET(socket_fd,&allreads);

    for (;;) {
        readmask = allreads;
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, NULL);
        if (rc <= 0)
            error(1, errno, "select failed");
        
        //当连接套接字上有数据可读，将数据读入到程序缓冲区
        if (FD_ISSET(socket_fd, &readmask)) {
            n = read(socket_fd, recv_line, MAXLINE);
            if (n < 0) {
                error(1, errno, "read error");
            } else if (n == 0) {
                error(1, 0, "server terminated \n");
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }
        //当标准输入上有数据可读，读入后进行判断
        if (FD_ISSET(0, &readmask)) {
            if (fgets(send_line, MAXLINE, stdin) != NULL) {
                if (strncmp(send_line, "shutdown", 8) == 0) {//比较两个字符串的前8位，相等为0
                    FD_CLR(0, &allreads);
                    if (shutdown(socket_fd, 1)) {
                        error(1, errno, "shutdown failed");
                    }
                } else if (strncmp(send_line, "close", 5) == 0) {
                    FD_CLR(0, &allreads);
                    if (close(socket_fd)) {
                        error(1, errno, "close failed");
                    }
                    sleep(6);
                    exit(0);
                } else {
                    int i = strlen(send_line);
                    if (send_line[i - 1] == '\n') {
                        send_line[i - 1] = 0;
                    }

                    printf("now sending %s\n", send_line);
                    size_t rt = write(socket_fd, send_line, strlen(send_line));
                    if (rt < 0) {
                        error(1, errno, "write failed ");
                    }
                    printf("send bytes: %zu \n", rt);
                }

            }
        }

    }

}