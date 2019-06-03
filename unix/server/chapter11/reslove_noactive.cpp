#include "header.h"
#include "lst_timer.h"
<<<<<<< HEAD
// #include <sys/epoll.h>
// #include <signal.h>
using namespace std;
/**
 * 使用alarm函数周期性地出发SIGALRM信号,关闭非活动的链接
 */
=======
>>>>>>> de84da81adf85044106848d2f40bdaf3b6bfb903

/**
 * 使用alarm函数周期性地出发SIGALRM信号,关闭非活动的链接
 */

int const FD_LIMIT = 65535;
int const MAX_EVENT_NUMBER = 1024;
int const TIMESLOT = 5;
static int pipefd[2];

<<<<<<< HEAD
/*利用链表来管理定时器*/
static sort_timer_lst timer_list;
static int epollfd = 0;

/**
 * 每个使用 ET 模式的 fd 都应该是非阻塞的.如果是阻塞的,那么读或者写操作会因为没有后续的
 * 事件而一直处于阻塞状态.
*/
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, new_option);
    return old_option;
}

/*把fd加入epoll监听中*/
void addfd(int epollfd, int fd)
{ 
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

/*信号的处理函数*/
void sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char*) &msg, 1, 0);
    errno = save_errno;
}

void addsig(int sig)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert( sigaction(sig, &sa, NULL) != -1);
}

/*定时器的回调函数,超时时调用.
其删除非活动连接 socket 上的注册事件,并且关闭之*/
void cb_func(client_data *user_data)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    cout << "close fd" << endl;
}

/*处理定时器任务*/
void timer_handler()
{
    timer_list.tick();
    /*一次 alarm 调用只会引起一次 SIGALRAM 信号,所以要重新定时,以不断出发SIGALARM 信号*/
    alarm(TIMESLOT);
}

int main(int argc, char* argv[])
{
    if(argc <= 2)
    {
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    /*建立连接*/
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    /*把监听的 fd 加入 epoll 监听事件*/
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd, listenfd);

    /*创建管道 fd 的 socket */
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    setnonblocking(pipefd[1]);
    addfd(epollfd, pipefd[0]);

    /*设置信号处理函数*/
    addsig(SIGALRM);
    addsig(SIGTERM);
    alarm(TIMESLOT); /*定时*/
    
    bool stop_server = false;
    epoll_event events[MAX_EVENT_NUMBER];
    client_data *users = new client_data[FD_LIMIT];
    bool timeout = false;
    /*处理连接*/
    while(!stop_server)
    {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if((number < 0) && (errno != EINTR))
        {
            cout << "epoll fail";
            break;
        }

        for(int i = 0; i < number; ++i)
        {
            /*处理监听的事件fd*/
            int sockfd = events[i].data.fd;
            /*若为新的客户连接请求*/
            if(sockfd == listenfd)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
                addfd(epollfd, connfd);
                /*设置客户信息*/
                users[connfd].address = client_address;
                users[connfd].sockfd = connfd;
                /*创建定时器,设置其回调函数和超时时间,然后绑定定时器和用户信息,并把定时器添加到
                链表timer_lst中*/
                util_timer *timer = new util_timer;
                timer->user_data = &users[connfd];
                timer->cb_func = cb_func;
                time_t cur = time(NULL);
                timer->expire = cur + 3 * TIMESLOT;
                users[connfd].timer = timer;
                timer_list.del_timer(timer);
            }
            /*信号事件的标志*/
            else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024];
                /*接受信号*/
                ret = recv(sockfd, signals, sizeof(signals), 0);
                if(ret == -1)
                {
                    // handler the error
                    cout << "signal received error!" << endl;
                    continue;
                }
                else if (ret == 0)
                {
                    continue;
                }
                else
                {
                    /*处理存在的信号*/
                    for(int i = 0; i < ret; ++i)
                    {
                        switch (signals[i])
                        {
                            case SIGALRM:
                            /*用timer变量标记有定时任务需要处理,但不立即处理.
                            因为定时任务的优先级不高*/
                                timeout = true;
                                break;
                            case SIGTERM:
                                stop_server = true;
                        }
                    }
                }
                
            }
            /*接收客户连接上的数据*/
            else if ( events[i].events & EPOLLIN)
            {
                memset(users[sockfd].buf, '\0', BUFFER_SIZE);
                ret = recv(sockfd, users[sockfd].buf, BUFFER_SIZE - 1, 0);
                cout << "get data :" << users[sockfd].buf << endl;
                
                /*若发生错误,或对方关闭连接,则我方关闭连接*/
                util_timer *timer = users[sockfd].timer;
                if(ret < 0)
                {
                    /*发生读错误,epoll不再监听sock,关闭sockfd,从定时器任务删除*/
                    if(errno != EAGAIN)
                    {
                        cb_func(&users[sockfd]);
                        if(timer)
                        {
                            timer_list.del_timer(timer);
                        }
                    }
                }
                else if (ret == 0)
                {
                    cb_func(&users[sockfd]);
                    if(timer)
                    {   
                        timer_list.del_timer(timer);
                    }                    
                }
                else
                {
                    /*若有数据可读,调整定时器时间,延长连接被关闭的时间*/
                    if(timer)
                    {
                        time_t cur = time(NULL);
                        timer->expire = cur + 3 * TIMESLOT;
                        cout << "adjust timer's time" << endl;
                        timer_list.add_timer(timer);
                    }
                }
            }
            else
            {
                // others
            }
        }
        /*最后处理定时事件,因为I/O优先级更高*/
        if(timeout)
        {
            timer_handler();
            timeout = false;
        }
    }
    /*关闭资源*/
    close(listenfd);
    close(pipefd[0]);
    close(pipefd[1]);
    delete[] users;
    return 0;
}
=======


>>>>>>> de84da81adf85044106848d2f40bdaf3b6bfb903
