#include "header.h"
using namespace std;

const int MAX_EVENT_NUMBER = 1024;
const int BUFFER_SIZE = 10;

// 把文件描述符设置为非阻塞的
int setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

// 把文件描述符fd上的刻度事件注册到epollfd指定的epoll内核时间表,参数enable_et指定
// 是否对fd采用et模式
void addfd(int epollfd, int fd, bool enable_et){
    epoll_event event;
    event.data.fd = fd;
    if(enable_et){
        // events描述事件类型
        event.events |= EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

/**
 * LT模式下的工作流程
 */
void lt(epoll_event* events, int number, int epollfd, int listenfd){
    char buf[BUFFER_SIZE];
    for(int i = 0;i < number; ++i){
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd){
            struct sockaddr_in client;
            socklen_t client_addrlength = sizeof(client);
            int connfd = accept(listenfd, (struct sockaddr*)&client, &client_addrlength);
            if(connfd < 0){
                cout << "error : " << errno << endl;
                break;
            }
            addfd(epollfd, connfd, false);
        }else if (events[i].events & EPOLLIN)
        {
            // 只要socket读缓存还有数据,此段代码被触发
            cout << "epoll trigger once" << endl;
            memset(buf, '\0', BUFFER_SIZE);
            int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
            if(ret <= 0){
                close(sockfd);
                continue;
            }
            cout << "get normal data :" << buf << endl;
        }else
        {
            cout << "something else happens " << endl;
        }
        
    }

}

/**
 * et模式的工作流程
 */
void et(epoll_event* events, int number, int epollfd, int listenfd){
    char buf[BUFFER_SIZE];
    for(int i = 0;i < number; ++i){
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd){
            struct sockaddr_in client;
            socklen_t client_addrlength = sizeof(client);
            int connfd = accept(listenfd, (struct sockaddr*)&client, &client_addrlength);
            if(connfd <= 0){
                cout << "error" << errno << endl;
                break;
            }
            addfd(epollfd, connfd, true);
        }else if (events[i].events & EPOLLIN)
        {
            cout << "event trigger once" << endl;
            // 因为这段代码只会在fd可读时被出发一次,故我们要死循环读完所有数据
            while(1){
                int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
                if(ret < 0){
                    // 对于非阻塞IO,以下条件代表数据已全部读取完毕.
                    if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
                        cout << "read later" << endl;
                        break;
                    }
                    close(sockfd);
                    break;
                }else if (ret == 0)
                {
                    close(sockfd);
                }else
                {
                    cout << "get message : " << buf << endl;
                }
                
            }
        }
    
    }
}


int main(int argc, char* argv[]){
    if(argc <= 2){
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in address;
    bzero( &address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    // 使用epoll
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd, listenfd, true);
    while(1){
        // 该函数成功时返回就绪的文件描述符的个数,并且就绪事件复制到events
        int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(ret < 0){
            cout << "epoll failed" << endl;
            break;
        }
        lt(events, MAX_EVENT_NUMBER, epollfd, listenfd);
        // et(events, MAX_EVENT_NUMBER, epollfd, listenfd);
    }
    close(listenfd);
    return 0;
}