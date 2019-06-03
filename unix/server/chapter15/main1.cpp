#include "../chapter14/locker.h"
#include "threadpool.h"
#include "http_conn.h"

using std::cout;
using std::endl;

static const int MAX_FD = 1024;
static const int MAX_EVENT_NUMBER = 10000;

extern int addfd( int epollfd, int sockfd, bool one_shot = true );
extern int removefd( int epollfd, int sockfd );

void addsig( int sig, void( handler ) (int), bool restart = true )
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

void show_error( int connfd, const char* info )
{
    cout << info << endl;
    send( connfd, info, strlen( info ), 0 );
    close( connfd );
}


int main( int argc, char* argv[] )
{
    if( argc <= 2 )
    {
        cout << argv[0] << endl;
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi( argv[2] );
    /* 忽略 SIGPIPE 信号 */
    addsig( SIGPIPE, SIG_IGN );

    /* 创建线程池 */
    threadpool< http_conn > *pool = NULL;
    try
    {
        pool = new threadpool< http_conn >;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    /* 预先为每个可能的客户连接分配一个 http_conn 对象 */
    http_conn *users = new http_conn[ MAX_FD ];
    assert( users );
    int user_count = 0;

    /* 创建主连接,监听 socket */
    int listenfd = socket( PF_INET, SOCK_STREAM, 0 );
    assert( listenfd >= 0 );
    struct linger tmp = { 1, 0 };
    /* 设置连接:如关闭时有未发送数据，则逗留。 */
    setsockopt( listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof( tmp ) );

    int ret = 0;
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr ); 
    address.sin_port = htons( port );

    ret = bind( listenfd, (struct sockaddr*)&address, sizeof( address ) );
    /* assert( ret != -1 ); */
    assert( ret >= 0 );

    ret = listen( listenfd, 5 );
    assert( ret >= 0 );

    epoll_event events[ MAX_EVENT_NUMBER ];

    int epollfd = epoll_create( 5 );
    assert( epollfd != -1 ); 
    /* 主连接用来监听客户的连接,不用设置 true  */
    addfd( epollfd, listenfd, false );
    /* addfd( epollfd, listenfd ); */
    http_conn::m_epollfd = epollfd;

    while( true )
    {
        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 );
        /* if( ( number < 0 ) && ( errno != EAGAIN ) ) */
	/* EINTR是linux中函数的返回状态，在不同的函数中意义不同。 表示某种阻塞的操作，被接收到的信号中断，造成的一种错误返回值。 */
	if( ( number < 0 ) && ( errno != EINTR ) )
        {
            cout << " epoll failed " << endl;
            break;
        }

        for( int i = 0; i < number; ++i )
	{
            int sockfd = events[i].data.fd;
            /* 有新连接 */
            if( sockfd == listenfd )
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
                if( connfd < 0 )
                {
                    cout << "error is " << errno << endl;
                    continue;
                }
                if( http_conn::m_user_count >= MAX_FD )
                {
                    show_error( connfd, "Internal server busy" );
                    continue;
                }
                /* 初始化客户连接 */
                /* users[ sockfd ].init( connfd, client_address ); */
		users[ connfd ].init( connfd, client_address );
            }
            /* EPOLLRDHUP:对端断开连接,或者对端关闭了写操作
               EPOLLHUB:挂起.比如管道的写端被关闭后,该端 fd 将接受到此事件 */
            else if( events[ i ].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ) )
            {
                /* 此三种情况均关闭连接 */
                users[ sockfd ].close_conn();
            }
            /* 可读 */
            else if ( events[ i ].events & EPOLLIN )
            {
                /* 根据读的结果来决定,是将任务添加到线程池或者是关闭连接 */
                if( users[ sockfd ].read() )
                {
                    pool->append( users + sockfd );
                }
                else
                {
                    users[ sockfd ].close_conn();
                }
                
            }
            /* 可写 */
            else if ( events[ i ].events & EPOLLOUT )
            {
                /* 根据写的结果决定是否关闭连接 */
                if( ! users[ sockfd ].write() )
                {
                    users[ sockfd ].close_conn();
                }
            }
	    else
	   {}
        }
    }
    close( epollfd );
    close( listenfd );
    delete [] users;
    delete pool;
    return 0;  
}
