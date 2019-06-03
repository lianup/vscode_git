#include "header.h"
using namespace std;

// 注意,linux标准库没有pthread.h,要自行编译添加lpthread
const int MAX_EVENT_NUMBER = 1024;
const int BUFFER_SIZE = 1024;
struct fds
{
    int epollfd;
    int sockfd;
};
/**
 * epolloneshot:保证同一时刻只有一个线程操纵socket
 * 保证机制:os最多触发fd上注册的一个可读or异常事件,且只触发一次.
 */
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
 * 重置fd上的事件.
 */
void reset_oneshot(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void* worker(void* arg)
{
    int sockfd = ((fds*)arg)->sockfd;
    int epollfd = ((fds*)arg)->epollfd;
    cout << "start the thread to receive data on fd :" << sockfd << endl;
    char buf[BUFFER_SIZE];
    while(1)
    {
        int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
        if(ret == 0)
        {
            close(sockfd); 
            cout << "foeriner closed the connection" << endl;
            break;
        }
        else if (ret < 0)
        {
            if(errno == EAGAIN){
                reset_oneshot(epollfd, sockfd);
                cout << "read later" << endl;
                break;
            }
        }
        else
        {
            cout << "get content" << buf << endl;
            // 模拟数据处理过程
            sleep(5);
        }
        
    }
    cout << "end thread receiving data on fd " << sockfd << endl;
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
        for(int i = 0;i < ret; ++i){
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd){
            struct sockaddr_in client;
            socklen_t client_addrlength = sizeof(client);
            int connfd = accept(listenfd, (struct sockaddr*)&client, &client_addrlength);
            if(connfd <= 0){
                cout << "error" << errno << endl;
                break;
            }
            // 为每个非监听fd都注册EPOLLONESHOT事件
            addfd(epollfd, connfd, true);
        }else if (events[i].events & EPOLLIN)
        {
            pthread_t thread;
            fds fds_for_new_worker;
            fds_for_new_worker.epollfd = epollfd;
            fds_for_new_worker.sockfd = sockfd;
            // 新开一个线程为sockfd服务
            pthread_create(&thread, NULL, worker, (void*)&fds_for_new_worker);
        }   
        else
        {
            cout << "something else happend" << endl;
        } 
    }    
    }
    close(listenfd);
    return 0;
}