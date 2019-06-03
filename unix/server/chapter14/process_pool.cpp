#include "header.h"
#ifndef PROCESSPOOL_H
#define PROCESSPOOL_H


/* 半同步,半异步进程池实现(主线层异步,处理线程同步) */

/* 描述一个子进程的类. m_pid:目标子进程的 pid , m_pipefd 是父进程和子进程通用的管道 */
class process
{
public:
    pid_t m_pid;
    int m_pipefd[2];

public:
    process() : m_pid(-1){}
};

/* 进程池类,模板参数是用来处理逻辑任务的类 */
template< typename T >
class processpool
{
private:
    /* 最大子进程数量 */
    static const int MAX_PROCESS_NUMBER = 16;
    /* 每个子进程最多能处理的客户数量 */
    static const int USER_PER_PROCESS = 65535;
    /* epoll 最多能处理的事件数 */
    staitc const int MAX_EVENT_NUMBER = 10000;
    /* 进程池中的进程总数 */
    int m_process_number;
    /* 进程池在池中的序号, 从 0 开始 */
    int m_idx;
    /* 每个进程都有一个 epoll 内核事件表,用 m_epollfd 标识 */
    int m_epollfd;
    /* 监听 socket */
    int m_listenfd;
    /* 子进程通过 m_stop 来决定是否停止运行 */
    int m_stop;
    /* 保存所有子进程的描述信息 */
    process *m_sub_porcess;
    /* 进程池静态实例 */
    static processpool< T > *m_instance;
    /* 保证单例的互斥锁(自加的) */
    static const pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

private:
    processpool( int listenfd, int process_number = 8 );

private:
    void setup_sig_pipe();
    void run_parent();
    void run_child();

public:
    /* 单例模式 */
    static processpool< T >* creat( int listenfd, int process_number = 8 )
    {

        if( !m_instance )
        {
            pthread_mutex_lock( &mutex );
            if( !m_instance )
            {
            m_instance = new processpool< T >( listenfd, process_number );
            }
            pthread_mutex_unlock( &mutex );
        }
        return m_instance;
    }
    ~processpool()
    {
        delete [] m_sub_process;
    }
    void run();
};

template< typename T >
processpool< T >* processpool< T >::m_instance = NULL;

/* 用于处理信号的管道 */
static int sig_pipefd[2];

static int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

static void addfd( int epollfd, int fd )
{   
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLERR;
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking( fd );
}

static void removefd( int epollfd, int fd )
{
    epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0 );
    close( fd );
}

static void sig_handler( int sig )
{
    int save_errno = errno;
    int msg = sig;
    send( sig_pipefd[1], (char*)&msg, 1, 0 );
    errno = save_errno;
}

static void addsig( int sig, void(handler )(int), bool restart = true )
{
    struct sigaction sa;
    memset( &sa, '\0', sizeof( sa ) );
    sa.sa_handler = handler;
    if( restart )
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset( &sa.sa_mask );
    assert( sigaction( sig, &sa, NULL ) != -1 );
}

/* 进程池构造函数 */
template< typename T >
processpool< T >::processpool( int listenfd, int process_number )
    :m_listenfd( listenfd ), m_process_number( process_number ), m_idx( -1 ), 
    m_stop( false )
{
    assert( ( process_number > 0 ) && ( process_number <= MAX_PROCESS_NUMBER ) );

    m_sub_porcess = new process[ process_number ];
    assert( m_sub_porcess );

    /* 创建子进程,并建立和父进程之间的管道 */
    for( int i = 0; i < process_number; ++i )
    {
        int ret = socketpair( PF_UNIX, SOCK_STREAM, 0, m_sub_porcess[ i ].m_pipefd );
        m_sub_porcess[ i ].m_pid = fork();
        assert( m_sub_porcess[ i ].m_pid >= 0 );
        if(m_sub_porcess[ i ].m_pid > 0)
        {
            close( m_sub_porcess[ i ].m_pipefd[ 1 ] );
            continue;
        }
        else
        {
            close( m_sub_porcess[ i ].m_pipefd[0] );
            m_idx = i;
            break;
        }
    }
}

/* 统一事件源 */
template< typename T > 
void processpool< T >::setup_sig_pipe()
{
    /* 创建 epoll 事件监听表和信号管道 */
    m_epollfd = epoll_create( 5 );
    assert( m_epollfd != -1 );

    int ret = socketpair( PF_UNIX, SOCK_STREAM, 0, m_epollfd );

    setnonblocking( sig_pipefd[1] );
    addfd( m_epollfd, sig_pipefd[0] );

    /* 设置信号处理函数 */
    addsig( SIGCHLD, sig_handler );
    addsig( SIGTERM, sig_handler );
    addsig( SIGINT, sig_handler );
    addsig( SIGPIPE, SIG_IGN );
}

/* 线程池运行函数,注意要通过 m_idx 判断接下来运行的是父进程代码还是子进程代码 */
template< typename T >
void processpool< T >::run()
{
    if( m_idx != -1 )
    {
        run_child();
        return;
    }
    run_parent();
}

template< typename T>
void processpool< T >::run_child()
{
    setup_sig_pipe();
    
    /* 每个子进程都要通过其在线程池中的序号值 m_idx 找到与父进程通信的管道 */
    int pipefd = m_sub_porcess[ m_idx ].m_pipefd[1];
    addfd( m_epollfd, pipefd );

    epoll_event events[ MAX_EVENT_NUMBER ];
    T *users = new T[ USER_PER_PROCESS ];
    assert( users );
    int number = 0;
    int ret = -1;
    
    while( !m_stop )
    {
        number = epoll_wait( m_epollfd, events, MAX_EVENT_NUMBER, -1 );
        if( ( number < 0 ) && ( errno != EAGAIN ) )
        {
            cout << "epoll faild " << endl;
        }
        for( int i = 0; i < number; ++i )
        {   
            int sockfd = events[i].data.fd;
            /* 父进程向子进程发送数据(信号) */
            if( ( sockfd == pipefd ) && ( events[i].data.fd & EPOLLIN ) ) 
            {
                /* 从管道接受数据,数据保存在 client 中,若读取成功,说明有新客户链接 */
                int client = 0;
                ret = recv( sockfd, ( char* )&client, sizeof( client ), 0 );
                if( ( ret < 0 ) && ( errno != EAGAIN ) || ret == 0 )
                {
                    continue;
                }
                else if( client == '1' )
                {
                    /* 建立与客户的连接 */
                    struct sockaddr_in client_address;
                    socklen_t client_addrlength = sizeof( client_address );
                    int connfd = accept( sockfd, ( struct sockaddr* )&client_address, &client_addrlength );
                    if( connfd < 0 )
                    {
                        cout << "connet with client error! errno is:" << errno << endl;
                        continue;
                    }
                    addfd( m_epollfd, sockfd );

                    /* 模板类 T 必须实现init方法,来初始化一个客户链接. */
                    users[ connfd ].init( m_epollfd, connfd, client_address );
                }
                /* 当父进程接收到终止的信号,向子进程发送消息,让子进程自行终止. */
                else if ( client == '2' )
                {
                    m_stop = true;
                    break;
                }       
            }
            /* 处理子进程接收到的信号 */
            else if ( ( sockfd == sig_pipefd[0] ) && ( events[i].events & EPOLLIN ) )
            {
                int sig;
                char signals[ 1024 ];
                ret = recv( sockfd, signals, 1024, 0 );
                if( ret < 0 )
                {
                    continue;
                }
                else
                {
                    for( int i = 0; i < ret; ++i )
                    {
                        switch ( signals[ i ] )
                        {
                            /* 子进程状态发生变化,退出或暂停 */
                            case SIGCHLD:
                                {
                                    pid_t pid;
                                    int stat;
                                    while( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 )
                                    {
                                        continue;
                                    }
                                    break;
                                }
                            /* 终止进程 */
                            case SIGTERM:
                            case SIGINT:
                            {
                                m_stop = true;
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
            /* 如果是其他可读数据,则必然是客户请求到来.调用逻辑处理对象的 process 方法处理 */
            else if ( ( event[i].event & POLLIN ) )
            {
                /* 这是 T 类型要实现的(客户) */
                users[ sockfd ].process();
            }
            else
            {
                continue;
            }
        }
    }

    delete [] users;
    close( pipefd );
    close( m_epollfd );
}

template< typename T >
void processpool< T >::run_parent()
{
    setup_sig_pipe();

    /* 父进程监听listenfd */
    addfd( m_epollfd, m_listenfd );

    epoll_event events[ EVENT_MAX_NUMBER ];
    int sub_process_counter = 0;
    int new_conn = 1;
    int stop = 2;
    int number = 0;
    int ret = -1;
    
    while( !m_stop )
    {
        int number = epoll_wait( m_epollfd, events, MAX_EVNET_NUMBER, -1 );
        if( ( number < 0 ) && ( errno != EAGAIN ) )
        {
            cout << "main porcess epoll error" << endl;
            break;
        }

        for( int i = 0; i < nubmer; ++i )
        {
            int sockfd = events[i].data.fd;
            if( sockfd == m_listenfd )
            {
                /* 若有新连接到来,采用 Round Robin 方式分配给一个子进程处理 */
                int i = sub_process_counter;
                do
                {
                    if( m_sub_porcess[i].m_pid != -1 )
                    {
                        break;
                    }
                    i = ( i + 1 ) % m_process_number;
                } while ( i != sub_process_counter );
                if( m_sub_porcess[ i ].m_pid == -1 )
                {
                    m_stop = true;
                    break;
                }
                sub_process_counter = ( i + 1 ) % m_process_number;
                send( m_sub_porcess[i].m_pipefd[0], (char *)&new_conn, sizeof( new_conn ), 0 );
                cout << "send request to child" << endl;
            }
            /* 处理父进程接受到的信号 */
            else if ( ( sockfd == sig_pipefd[0] ) && ( events[i].events & EPOLLIN ) )
            {
                int sig;
                char signals[ 1024 ];
                ret = recv( sig_pipefd[0], signals, 1024, 0 );
                if( ret < 0 )
                {
                    continue;
                }
                else
                {
                    for( int i = 0; i < ret; ++i )
                    {
                        switch ( signals[i] )
                        {
                            case SIGCHLD:
                                pid_t pid;
                                int stat;
                                while( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 )
                                {
                                    /* 有退出的子进程,并关闭相应的管道, 设置 m_pid = -1
                                    来标记子进程已经退出 */
                                    for( int i = 0; i < m_process_number; ++i )
                                    {
                                        if( m_sub_porcess[ i ].m_pid == -1 )
                                        {
                                            cout << "child " << i << "join" << endl;
                                            close( m_sub_porcess[ i ].m_pipefd[0] );
                                            m_sub_porcess[ i ].m_pid = -1;
                                        }
                                    }
                                }
                                /* 此时所有的子进程已退出,关闭主进程 */
                                m_stop = true;
                                for( int i = 0; i < m_process_number; ++i )
                                {
                                    if( m_sub_porcess[ i ].m_pid != -1 )
                                    {
                                        m_stop = false;
                                    }
                                }
                                break;
                            case SIGTERM:
                            case SIGINT:
                            /* 若父进程接受到终止信号,就杀死所有的子进程,并等待它们全部都结束.
                            更好的办法:管道发送特殊信号( 自行实现 ) */
                            cout << "kill all childrens now " << endl;
                            for( int i = 0; i < m_process_number; ++i )
                            {
                                int pid = m_sub_porcess[ i ].m_pid;
                                if( pid != -1 )
                                {
                                    kill( pid, SIGTERM );
                                }
                            }
                            /* 父进程向管道发送数据通知子进程自己关闭的实现 */
                            // for( int i = 0; i < m_process_number; ++i )
                            // {
                            //     int pid = m_sub_porcess[ i ].m_pid;
                            //     if( pid != -1 )
                            //     {

                            //         send( m_sub_porcess[ i ].m_pipefd[0], (char*)&stop, 
                            //         sizeof( stop ), 0  );
                            //         cout << "send stop message to child " << endl;
                            //     }
                            // }

                            //  while( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 )
                            // {
                            //     /* 有退出的子进程,并关闭相应的管道, 设置 m_pid = -1
                            //     来标记子进程已经退出 */
                            //     for( int i = 0; i < m_process_number; ++i )
                            //     {
                            //         if( m_sub_porcess[ i ].m_pid == -1 )
                            //         {
                            //             cout << "child " << i << "join" << endl;
                            //             close( m_sub_porcess[ i ].m_pipefd[0] );
                            //             m_sub_porcess[ i ].m_pid = -1;
                            //         }
                            //     }
                            // }
                            // m_stop = true;
                            // /* 检查一遍,如果此时还是有子进程未关闭,则强行关闭(虽然不可能) */
                            // for( int i = 0; i < sub_process_number; ++i )  
                            // {
                            //     if( m_sub_porcess[ i ].m_pid != -1)
                            //     {
                            //         kill( m_sub_porcess[ i ].m_pid, SIGTERM );
                            //     }
                            // }                            
                            break;

                            default:
                                break;
                        }
                    }
                }                                
            }
            else
            {
                continue;
            }           
        }
    }
    close( m_listenfd ); /* 谁创建的谁关闭 */
    close( m_epollfd );
}


#endif // !PROCESSPOLL_H#define PROCESSPO