#include<stdio.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<netinet/in.h>
#include<string.h>
#include<error.h>
#include<errno.h>
#include<signal.h>
#include<unistd.h>
#include<malloc.h>
#include<stdlib.h>


#define SERV_PORT 43211
#define LISTENQ 1024
#define MAXLINE 4096

static int count;

static void sig_int(int signo){
    printf("\nrecvived %d datagrams\n",count);
    exit(0);
}

char *run_cmd(char *cmd){
    char *data=malloc(16384);
    bzero(data,sizeof(data));
    FILE *fdp;
    const int max_buffer=256;
    char buffer[max_buffer];
    fdp=popen(cmd,"r");
    char *data_index=data;
    if(fdp){
        while(!feof(fdp)){
            if(fgets(buffer,max_buffer,fdp)!=NULL){
                int len=strlen(buffer);
                memcpy(data_index,buffer,len);
                data_index+=len;
            }
        }
        pclose(fdp);
    }
    return data;
}

int main(){

    //1. 调用socket函数，创建套接字描述符
    int listenfd=socket(AF_INET,SOCK_STREAM,0);
    //2. 初始化套接字地址信息，设置服务端ip地址和端口号
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(SERV_PORT);
    //3. 调用bind函数将套接字地址和套接字描述符绑定
    int rt1=bind(listenfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    if(rt1<0){
        error(1,errno,"bind failed");
    }
    //4. 调用listen函数,将主动套接字转化为监听套接字
    int rt2=listen(listenfd,LISTENQ);
    if(rt2<0){
        error(1,errno,"listen failed");
    }
    //5. 
    signal(SIGINT,sig_int);
    signal(SIGPIPE,SIG_IGN);
    //6. 调用accept函数，返回监听描述符
    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len=sizeof(client_addr);

    char message[MAXLINE];
    count=0;

    for(;;){
        if((connfd=accept(listenfd,(struct sockaddr *)&client_addr,&client_len))<0){
            error(1,errno,"bind failed");
        }
        for(;;){
            //调用read函数接收客户端的数据
            int n=read(connfd,message,MAXLINE);
            if(n<0){
                error(1,errno,"error read");
            }else if(n==0){
                printf("client closed \n");
                close(connfd);
                break;
            }
            message[n]=0;
            printf("clinet enter cmd %s\n",message);
            char send_line[MAXLINE];
            if(strncmp(message,"pwd",3)==0){
                char buf[256];
                char *send_line;
                send_line=getcwd(buf,256);
                if(send(connfd,send_line,strlen(send_line),0)<0){
                    error(1,errno,"error write");
                }
            }else if(strncmp(message,"cd ",3)==0){
                char *path=message+3;
                char *result=getenv(path);
                if(chdir(result)<0){
                    printf("change dir failed!!!\n");
                    break;
                }
                char *send_line=run_cmd("cd");
                if(send(connfd,send_line,strlen(send_line),0)<0){
                    error(1,errno,"error write");
                }
            }else if(strncmp(message,"ls",2)==0){
                char *send_line=run_cmd("ls");
                if(send(connfd,send_line,strlen(send_line),0)<0){
                    error(1,errno,"error write");
                }
            }else{
                char send_line[MAXLINE]="error:unknow input type!!!";
                if(send(connfd,send_line,strlen(send_line),0)<0){
                    error(1,errno,"error write");
                }
            }
            count++;
        }
    }
}