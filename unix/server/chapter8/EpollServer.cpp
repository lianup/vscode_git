#include "header.h"
using namespace std;
const int USER_LIMIT = 5;
const int BUFFER_SIZE = 64;
const int FD_LIMITE = 65535;
const int MAX_EVENT_NUM = 5;

/**
 * 改成 Epoll 来实现
 */


/**
 * 客户信息:socket地址\待写到客户端的数据的位置\从客户端读入的数据
 */
struct client_data{
    sockaddr_in address;
    char* write_buf;
    char buf[BUFFER_SIZE];
};

int setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, new_option);
    return old_option;
}


void addfd( int epollfd, int sockfd, bool one_shot )
{
    epoll_event event;
    event.data.fd = sockfd;
    /* 设置为可读事件, LT 模式, socket 断开连接事件 */
    /* event.events = EPOLLIN | EPOLLET | EPOLLHUP; */
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    /* 这个操作得在后面 */
    /* epoll_ctl( epollfd, EPOLL_CTL_ADD, sockfd, &event ); */
    if( one_shot )
    {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, sockfd, &event );
    setnonblocking( sockfd );
}



/**
 * 服务器使用 epoll 同时管理监听socket和连接socket
 */
int main(int argc,char* argv[]){
    if(argc <= 2){
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

    // 创建user数组,分配FD_LIMIT个client_data对象:每个socket都可以获得一个对象,并且
    // socket的值可以用来直接索引socket链接对应的client_data
    client_data* users = new client_data[FD_LIMITE]; 
    
    epoll_event events[ MAX_EVENT_NUM ];
    int epoll_fd = epoll_create( 5 );
    assert( epoll_fd != -1 );
    addfd( epoll_fd, listenfd, false );
    
    int user_counter = 0;

    while(true)
    {
        int number = epoll_wait(epoll_fd, events, MAX_EVENT_NUM, -1);
        if( ( number < 0 ) && ( errno != EINTR ) )
        {
            cout << " epoll failed " << endl;
            break;
        }
        for( int i = 0;i < number; ++i )
        {
            int sockfd = events[i].data.fd;
            /* 创建新连接 */
            if( sockfd == listenfd )
            {
                // 创建客户连接
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength
                );
                if(connfd < 0)
                {
                    cout << "connect faild" << endl;
                    continue;
                }
                // 如果请求太多,则关闭链接
                if(user_counter >= USER_LIMIT)
                {
                    const char* info = "too many users";
                    cout << info << endl;
                    send(connfd, info, strlen(info), 0);
                    close(connfd);
                    continue;
                }
                // 否则添加新用户,同时修改fds和users数组
                ++user_counter;
                users[connfd].address = client_address;
                setnonblocking(connfd);
                addfd( epoll_fd, connfd, true );
                cout << "comes a new user : " << user_counter << endl;
            }
            else if (events[i].events & POLLERR)
            {
                cout << "get an error from " << user_counter << endl;
                char errors[100];
                memset(errors, '\0', sizeof(errors));
                socklen_t length = sizeof(errors);
                if(getsockopt(events[i].data.fd, SOL_SOCKET, SO_ERROR, &errors, &length) < 0)
                {
                    cout << "get socket option faild" << endl;
                }
                continue;
            }
            // 客户端关闭了链接
            else if (events[i].events & POLLRDHUP)
            {
                epoll_ctl( epoll_fd, EPOLL_CTL_DEL, sockfd, 0 );
                close( sockfd );
                --user_counter;
                cout << "a client left" << endl;
            
            }
            // 客户端发送了消息
            else if (events[i].events & POLLIN)
            {
                int connfd = events[i].data.fd;
                memset(users[connfd].buf, '\0', BUFFER_SIZE);
                ret = recv(connfd, users[connfd].buf, BUFFER_SIZE - 1, 0);
                cout << "get client data from " << connfd << ":" << users[connfd].buf << endl;
                if(ret < 0){
                    // 读操作出错,则关闭链接
                    if(errno != EAGAIN)
                    {
                        epoll_ctl( epoll_fd, EPOLL_CTL_DEL, connfd, events );
                        close( connfd );
                        --user_counter; 
                    }
                }
                else if (ret == 0)
                {
                    
                }
                else
                {
                    // 接受正常,则通知其他客户端写数据
                    for(int j = 1;j <= user_counter; ++j){
                        if(events[j].data.fd == connfd){
                            continue;
                        }
                        /* 感觉这里不太对,应该是要遍历所有用户 */
                        //  events[j].events |= ~POLLIN;
                        // events[j].events |= POLLOUT;
                        // epoll_ctl( epoll_fd, EPOLL_CTL_MOD, events[j].data.fd, &events[j] );
                        // users[events[j].data.fd].write_buf = users[connfd].buf;
                        send( connfd, users[connfd].buf, sizeof( users[connfd].buf ), 0 );
                    }
                }  
            }
            //  如果客户端写就绪,那就向其写入数据
            else if (events[i].events & POLLOUT)
            {
                int connfd = events[i].data.fd;
                if(!users[connfd].write_buf){
                    continue;
                }
                ret = send(connfd, users[connfd].write_buf, strlen(users[connfd].write_buf), 0);
                users[connfd].write_buf = NULL;
                // 写完数据后需要重新注册fds[i]上的可读事件
                // events[i].events = ~POLLOUT;
                // fds[i].events = POLLIN;
            }
        }
    }
    delete[] users;
    close(listenfd);
    return 0;
}
