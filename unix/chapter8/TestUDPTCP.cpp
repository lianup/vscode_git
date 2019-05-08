#include "header.h"
using namespace std;

/**
 * 同时处理TCP和UDP服务(用epoll监听两个sockt,流和数据报socket)
 */

const int MAX_EVENT_NUMBER = 1024;
const int TCP_BUFFER_SIZE = 512;
const int UPD_BUFFER_SIZE = 1024;

int setnoblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int addfd(int epollfd, int fd){

    epoll_event event;
    event.events = EPOLLIN | EPOLLERR;
    event.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnoblocking(fd);
}


int main(int argc, char* argv[]){
    if(argc <= 2){
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    int ret = 0;
    address.sin_port = htons(port);
    // 创建tcpsocket并绑定到port上
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    // 创建UDP socket,并绑定到端口上
        bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int udpfd = socket(PF_INET, SOCK_DGRAM, 0);
    assert(udpfd >= 0);
    ret = bind(udpfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    // 注册TCP UPD上的可读事件
    addfd(epollfd, listenfd);
    addfd(epollfd, udpfd);

    // 使用epoll监听到事件后发送数据
    while(1)
    {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(number < 0){
            cout << "epoll failed" << endl;
            break;
        }
        for(int i = 0;i < number; ++i)
        {
            int sockfd = events[i].data.fd;
            // 如果是TCP端口可读,则要建立连接
            if(sockfd == listenfd)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(sockfd, (struct sockaddr*)&client_address, &client_addrlength);
                addfd(epollfd, connfd);
            }
            // 如果是UDP连接,则不用建立连接,直接返回数据
            else if (sockfd == udpfd)
            {
                char buf[UPD_BUFFER_SIZE];
                memset( buf, '\0', UPD_BUFFER_SIZE);
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);

                ret = recvfrom( sockfd, buf, UPD_BUFFER_SIZE - 1, 0, 
                (struct sockaddr*)&client_address, &client_addrlength);
                if(ret > 0)
                {
                    sendto(sockfd, buf, UPD_BUFFER_SIZE - 1, 0, 
                    (struct sockaddr*)&client_address, client_addrlength);
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                char buf[TCP_BUFFER_SIZE];
                memset(buf, '\0', TCP_BUFFER_SIZE);
                ret = recv(sockfd, buf, TCP_BUFFER_SIZE - 1, 0);
                if(ret < 0)
                {
                    // 表示客户端断开了连接
                    if((errno == EAGAIN) || (errno == EWOULDBLOCK))
                    {
                        break;
                    }
                    close(sockfd);
                    break;
                }
                else if (ret == 0)
                {
                    close(sockfd);
                }
                else
                {
                    send(sockfd, buf, TCP_BUFFER_SIZE - 1, 0);
                }
            }
            else
            {
                cout << "something else happen" << endl;
            }
            
        }
    }
    close(listenfd);
    return 0;
}