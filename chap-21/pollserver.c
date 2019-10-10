#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<error.h>
#include<errno.h>
#include<sys/poll.h>
#include<unistd.h>

#define SERV_PORT 43211
#define LISTENQ 1024
#define INIT_SIZE 128
#define MAXLINE 4096

int main(int argc,char **argv){
    int listen_fd,connected_fd;
    listen_fd=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in server_addr;
    __bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(SERV_PORT);
    //3. 调用bind函数，将套接字和套接字地址信息进行绑定
    int rt1=bind(listen_fd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    if(rt1<0){
        error(1,errno,"bind failed");
    }
    //4. 调用listen函数，
    int rt2=listen(listen_fd,LISTENQ);
    if(rt2<0){
        error(1,errno,"listen failed");
    }

    struct pollfd event_set[INIT_SIZE];
    event_set[0].fd=listen_fd;
    event_set[0].events=POLLRDNORM;

    for(int i=1;i<INIT_SIZE;++i){
        event_set[i].fd=-1;
    }
    int ready_number;
    char buf[MAXLINE];
    struct sockaddr_in client_addr;
    for(;;){
        if((ready_number=poll(event_set,INIT_SIZE,-1))<0){
            error(1,errno,"poll failed\n");
        }

        if(event_set[0].revents & POLLRDNORM)//若两个相等，得到的结果不变，不相等，
        {
            socklen_t client_len=sizeof(client_addr);
            connected_fd=accept(listen_fd,(struct sockaddr *)&client_addr,client_len);
            printf("connect a client\n");
            int i;
            for(i=1;i<INIT_SIZE;++i){
                if(event_set[i].fd<0){
                    event_set[i].fd=connected_fd;
                    event_set[i].events=POLLRDNORM;
                    break;
                }
            }

            if(i == INIT_SIZE){
                error(1,errno,"can not hold so many clients\n");
            }

            if(--ready_number<=0)
                continue;
        }

        for(int i=1;i<INIT_SIZE;++i){
            int socket_fd;
            if((socket_fd=event_set[i].fd)<0){
                continue;
            }
            ssize_t n;
            if(event_set[i].revents & (POLLRDNORM | POLLERR)){
                if((n=read(socket_fd,buf,MAXLINE))>0){
                    if(write(socket_fd,buf,n)<0){
                        error(1,errno,"write error\n");
                    }
                }else if(n==0 || errno == ECONNRESET){
                    close(socket_fd);
                    event_set[i].fd=-1;
                }else {
                    error(1,errno,"read error \n");
                }

                if(--ready_number<=0){
                    break;
                }
            }
        }
    }
}