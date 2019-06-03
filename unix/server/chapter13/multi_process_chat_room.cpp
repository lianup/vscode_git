#include "header.h"
using std::cout;
using std::endl;

/*多进程的聊天室服务器程序,所有客户 socket 链接的读缓冲设计为共享
注意:编译时要加上 -lrt ,因为 rt 是运行时库,提供了一些额外的函数定义*/

const int USER_LIMIT = 5;
const int BUFFER_SIZE = 1024;
const int FD_LIMIT = 65535;
const int MAX_EVENT_NUMBER = 1024;
const int PROCESS_LIMIT = 65536;

/* 处理客户连接的信息 */
struct client_data
{
    sockaddr_in address;
    int connfd;
    pid_t pid; /* 处理该连接的子进程的 PID */
    int pipefd[2]; /* 和父进程通信用的管道 */
};

static const char *shm_name = "/my_shm"; /* 指定要创建/打开的共享内存对象 */
int sig_pipefd[2];
int epollfd;
int listenfd;
int shmfd;
char *share_mem = 0;
/* 客户连接数组.进程用客户连接的编号来索引此数组 */
client_data *users = 0;
/* 子进程和客户连接的映射关系表.用进程的 PID 来索引次数组,可获得该进程所处理的客户连接的编号 */
int *sub_process = 0;
int user_count = 0;
bool stop_child = false;

int setnonblocking( int fd )
{
    int old_potion = fcntl( fd, F_GETFL );
    int new_option = old_potion | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_potion;
}

void addfd( int epollfd, int fd )
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}

void addsig( int sig, void(*handler)(int), bool restart = true )
{
    struct sigaction sa;
    memset( &sa, '\0', sizeof(sa) );
    sa.sa_handler = handler;
    if( restart )
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset( &sa.sa_mask );
    assert( sigaction( sig, &sa, NULL ) != -1 );
}

/* 当子进程为某信号,发送该信号给主进程(放在管道中) */
void sig_handler( int sig )
{
    int save_errno = errno;
    int msg = sig;
    send( sig, &msg, 1, 0 );
    errno = save_errno;
}

/* 停止子进程的函数 */
void child_term_handler( int sig )
{
    stop_child = true;


}

/* 子进程运行函数
参数 idx 指该子进程处理的客户连接的编号, share_mem 指出共享内存的起始地址 */
int run_child( int idx, client_data *users, char *share_mem )
{
    epoll_event events[ MAX_EVENT_NUMBER ];
    /* 子进程使用 I/O 复用技术来同时监听两个 fd :客户连接 socket ,与父进程通信的
     fd */
     int child_epollfd = epoll_create( 5 );
     assert( child_epollfd != -1 );
     int connfd = users[idx].connfd;
     addfd( child_epollfd, connfd );
     int pipefd = users[idx].pipefd[1];
     addfd( child_epollfd, pipefd );
     int ret;

     /* 子进程设置自己的信号处理函数 */
     addsig( SIGTERM, child_term_handler, false );

     while( !stop_child )
     {
         int number = epoll_wait( child_epollfd, events, MAX_EVENT_NUMBER, -1 );
         if( ( number < 0 ) && ( errno != EINTR ) )
         {
             cout << "child epoll failed" << endl;
             break;
         }

         for( int i = 0; i < number; ++i )
         {
             int sockfd = events[i].data.fd;
             /* 本进程负责的客户连接有数据到达 */
             if( (sockfd == connfd) && ( events[i].events & EPOLLIN ) )
             {
                 /*  把客户数据读取到对应的读缓存中 */
                 memset( share_mem + idx*BUFFER_SIZE, '\0', BUFFER_SIZE );
                 ret = recv( connfd, share_mem+idx*BUFFER_SIZE, BUFFER_SIZE-1, 0 );
                 if( ret < 0 )
                 {
                     if( errno != EAGAIN )
                     {
                        stop_child = true;
                     }
                 }
                 else if ( ret == 0 )
                 {
                    stop_child = true;
                 }
                 else
                 {
                     /* 成功读取客户数据后就通知主进程 (通过管道) 来处理 */
                     send( pipefd, ( char* )&idx, sizeof(idx), 0 );
                 }
             }
             /* 主进程通知本进程 (通过管道) 将第 client 个客户的数据发送到本进程负责的客户端 */
             else if ( ( sockfd == pipefd ) && ( events[i].events & EPOLLIN ) )
             {
                 int client = 0;
                 /* 接受主进程发来的数据,即有客户数据到达的连接编号 */
                 ret = recv( sockfd, (char*)&client, sizeof( client ), 0 );
                 if( ret < 0 )
                 {
                     if( errno != EAGAIN )
                     {
                         stop_child = true;
                     }
                 }
                 else if( ret == 0 )
                 {
                     stop_child = true;
                 }
                 else
                 {
                     /* 把数据放进共享内存段中的 client 使用的部分 */
                     send( connfd, share_mem + client * BUFFER_SIZE, BUFFER_SIZE, 0 );
                 }
             }
             else
             {
                 continue;
             }
         }
     }
     close( connfd );
     close( pipefd );
     close( child_epollfd );
     return 0;
}

void del_resouce()
{
    close( sig_pipefd[0] );
    close( sig_pipefd[1] );
    close( listenfd );
    close( epollfd );
    shm_unlink( shm_name );
    delete [] users;
    delete [] sub_process;
}

int main( int argc, char *argv[] )
{
    if(argc <= 2)
    {
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi( argv[2] );

    int ret = 0;
    struct sockaddr_in address; 
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( port );

    listenfd = socket( PF_INET, SOCK_STREAM, 0 );
    assert( listenfd >= 0 );

    ret = bind( listenfd, (struct sockaddr*)&address, sizeof(address) );
    assert(ret != -1);

    ret = listen( listenfd, 5 );
    assert( ret != -1 );

    /* 初始化用户数据结构 */
    user_count = 0;
    users = new client_data[ USER_LIMIT + 1 ];
    sub_process = new int [ PROCESS_LIMIT ];
    for( int i = 0; i < PROCESS_LIMIT; ++i )
    {
        sub_process[i] = -1;
    }

    /* 初始化 epoll 数据结构,并把主线程的 fd 注册进去 */
    epoll_event events[ MAX_EVENT_NUMBER ];
    epollfd = epoll_create( 5 );
    assert( epollfd != -1 );
    addfd( epollfd, listenfd );
    
    /* 创建双向通信管道,并注册管道 fd */
    ret = socketpair( PF_UNIX, SOCK_STREAM, 0, sig_pipefd );
    assert( ret != -1 );
    setnonblocking( sig_pipefd[1] );
    addfd( epollfd, sig_pipefd[0] );

    /* 添加需要处理的信号 */
    addsig( SIGCHLD, sig_handler );
    addsig( SIGTERM, sig_handler );
    addsig( SIGINT, sig_handler );
    addsig( SIGPIPE, SIG_IGN );
    bool stop_server = false;
    bool terminate = false;

    /* 创建共享内存,作为所有客户 socket 连接的读缓存 */
    shmfd = shm_open( shm_name, O_CREAT | O_RDWR, 0666 );
    assert( shmfd != -1 );
    ret = ftruncate( shmfd, USER_LIMIT*BUFFER_SIZE );
    assert( ret != -1 );

    share_mem = (char*)mmap( NULL, USER_LIMIT * BUFFER_SIZE, PROT_READ
| PROT_WRITE, MAP_SHARED, shmfd, 0 );
    assert( share_mem != MAP_FAILED );
    close( shmfd );

    while( !stop_server )
    {
        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 );
        if( ( number < 0 ) && ( errno != EINTR ) )
        {
            cout << "epoll failed " << endl;
        }
        for( int i = 0; i < number; ++i )
        {
            int sockfd = events[i].data.fd;
            /* 处理新的客户连接 */
            if( sockfd == listenfd )
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept( listenfd, (struct sockaddr*)&client_address, 
                &client_addrlength );
                
                if( sockfd < 0 )
                {
                    cout << "error is " << errno << endl;
                }
                /* 连接数过多,关闭连接 */
                if( user_count >= USER_LIMIT )
                {
                    const char *info = "too many users\n";
                    cout << info;
                    send( connfd, info, strlen( info ), 0 );
                    close( connfd );
                    continue;
                }
                /* 保存第 user_count 个客户连接的相关信息 */
                users[user_count].address = client_address;
                users[user_count].connfd = connfd;
                
                /* 在主和子进程间建立管道以传递数据 */
                ret = socketpair( PF_UNIX, SOCK_STREAM, 0, users[user_count].pipefd );
                assert( ret != -1 );
                
                /* 建立处理该连接的子进程 */
                pid_t pid = fork();
                if( pid < 0 )
                {
                    close( connfd );
                    continue;
                }
                /* 建立成功,目前为子进程在执行 */
                else if ( pid == 0 )
                {
                    /* 疑问:为啥子进程运行要把所有 fd 关闭了?因为这是从主进程copy过来的,
                    它需要. */
                    close( epollfd );
                    close( listenfd );
                    close( users[user_count].pipefd[0] );
                    close( sig_pipefd[0] );
                    close( sig_pipefd[1] );
                    run_child( user_count, users, share_mem );
                    /* 子进程用完就要释放掉 */
                    munmap( (void*)share_mem, USER_LIMIT*BUFFER_SIZE );
                    exit( 0 );
                }
                /* 为主进程 */
                else
                {
                    close( connfd );
                    close( users[user_count].pipefd[0] );
                    addfd( epollfd, users[user_count].pipefd[0] );
                    users[user_count].pid = pid;
                    sub_process[pid] = user_count;
                    ++user_count;
                }
            }
            /* 处理信号事件 */
            else if ( (sockfd == sig_pipefd[0]) && ( events[i].events & EPOLLIN ) )
            {
                int sig;
                char signals[1024];
                ret = recv( sig_pipefd[0], signals, sizeof(signals), 0 );
                if(ret == -1)
                {
                    continue;
                }
                else if ( ret == 0 )
                {
                    continue;
                }
                else
                {
                    for( int i = 0; i < ret; ++i )
                    {
                        switch ( signals[i] )
                        {
                            /* 子进程退出,表示某个客户关闭了连接 */
                            case SIGCHLD:
                                {
                                    pid_t pid;
                                    int stat;
                                    /* waitpid()会暂时停止目前进程的执行，直到有信号来到或子进程结束。 */
                                    while( ( pid = waitpid( -1, &stat, WNOHANG)) > 0 )
                                    {
                                        /* 用子进程的 pid 取得被关闭的客户连接的编号 */
                                        int del_user = sub_process[pid];
                                        sub_process[pid] = -1;
                                        if( ( del_user < 0 ) || ( del_user > USER_LIMIT ) )
                                        {
                                            continue;
                                        }
                                        /* 清除第 del_user 个客户连接使用的相关数据 */
                                        epoll_ctl( epollfd, EPOLL_CTL_DEL, users[del_user].pipefd[0], 0 );
                                        close( users[del_user].pipefd[0] );
                                        users[del_user] = users[--user_count];
                                        sub_process[users[del_user].pid] = del_user; /*??*/
                                    }
                                    if( terminate && user_count == 0 )
                                    {
                                        stop_server = true;
                                    }
                                    break;
                                }
                            case SIGTERM:
                            case SIGINT:
                                {
                                    /* 结束服务器程序 */
                                    cout << "kill all the child now" << endl;
                                    if( user_count == 0 )
                                    {
                                        stop_server = true;
                                        break;
                                    }
                                    for( int i = 0; i < user_count; ++i )
                                    {
                                        int pid = users[i].pid;
                                        kill( pid, SIGTERM );
                                    }
                                    terminate = true;
                                    break;
                                }
                            default:
                            {
                                break;
                            }
                        }
                    }
                }                
            }
            /* 某个子进程向父进程写入了数据 */
            else if ( events[i].events & EPOLLIN )
            {
                int child = 0;
                ret = recv( sockfd, ( char* )&child, sizeof( child ), 0 );
                cout << "read data from child accros pipe" << endl;
                if( ret == -1 )
                {
                    continue;
                }
                else if ( ret == 0 )
                {
                    continue;
                }
                else
                {
                    /* 向除了负责处理 child 个客户连接的子进程之外的其他子进程发送消息
                    ,通知它们有客户数据要写 */
                    for( int j = 0; j < user_count; ++j )
                    {
                        if( users[j].pipefd[0] != sockfd )
                        {
                            cout << "send data to child accross pipe" << endl;
                            send( users[j].pipefd[0], ( char* )&child, sizeof( child ), 0 );
                        }
                    }
                }
            }
        }
    }
    del_resouce();
    return 0;
}