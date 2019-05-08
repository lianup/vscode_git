#include "header.h"
using namespace std;

/**
 * 统一处理信号和I/O事件的一个简单实现
 */

const int MAX_EVENT_NUMBER = 1024;
static int pipefd[2];

int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, new_option);
    return old_option;
}

void addfd(int epollfd, int fd)
{
    epoll_event events;
    events.data.fd = fd;
    events.events = EPOLLIN | EPOLLERR;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &events);
}

/**
 * 信号处理函数
 */
void sig_handle(int sig)
{
    // 保留原来的errno,在函数最后恢复,以保证函数的可重入性
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char*)&msg, 1, 0);
    errno = save_errno;
}

void addsig(int sig)
{
    // sigaction 描述了信号处理的细节
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    // sa.__sigaction_handler = &sig_handle;
    sa.sa_flags |= SA_RESTART;
    // 在信号集中设置所有信号
    sigfillset( &sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int main(int argc,char* argv[])
{
    if(argc <= 2)
    {
        return 1;
    }
    const char* ip =argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET,SOCK_STREAM, 0);
    assert(listen >= 0);

    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd, pipefd[0]);

    // 使用socketpair创建管道,注册pipefd[0]上的可读事件
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    setnonblocking(pipefd[1]);
    addfd(epollfd, pipefd[0]);

    // 设置一些信号的处理函数
    addsig(SIGHUP); // 控制终端挂起
    addsig(SIGCHLD); // 子进程状态发生变化(暂停或者退出)
    addsig(SIGTERM); // 终止进程 
    addsig(SIGINT); // 键盘输入以中断进程
    bool stop_server = false;
    
    while(!stop_server)
    {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if((number < 0) && (errno != EINTR))
        {
            cout << "epoll failure" << endl;
            break;
        }
        for(int i = 0;i < number; ++i)
        {  
            int sockfd = events[i].data.fd;
            // 若就绪的fd是listenfd,则处理新的连接
            if(sockfd == listenfd)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(sockfd, (struct sockaddr*)&client_address, &client_addrlength);
                addfd(epollfd, connfd);
            }
            // 若就绪的文件描述符是pipefd[0], 则处理信号
            else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024];
                ret = recv(sockfd, signals, sizeof(signals), 0);
                if(ret == -1)
                {
                    continue;
                }
                else if (ret == 0)
                {
                    continue;
                }
                else
                {
                    // 因为每个信号值占1字节,所以按字节来逐个接受信号.
                    for(int i = 0;i < ret;++i)
                    {
                        switch (signals[i])
                        {
                            case SIGCHLD:
                            case SIGHUP:
                            {
                                continue;
                            }
                            case SIGTERM:
                            case SIGINT:
                            {
                                stop_server = true;
                            }
                        }
                    }
                }
                
            }
            else
            {   
            }
        }
    }
    cout << "close fds" << endl;
    close(listenfd);
    close(pipefd[1]);
    close(pipefd[0]);
    return 0;
}

