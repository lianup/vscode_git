#include "header.h"
using std::cout;
using std::endl;

/* 在两个进程中传递文件描述符(利用管道) */

static const int CONTROL_LEN = CMSG_LEN( sizeof(int) );

int recv_fd( int fd )
{
    struct iovec iov[1];
    struct msghdr msg;
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    cmsghdr cm;
    msg.msg_control = &cm;
    msg.msg_controllen = CONTROL_LEN;

    /* 专门用来接受 fd 的系统调用 */
    recvmsg( fd, &msg, 0 );
    
    int fd_to_read = *(int *)CMSG_DATA( &cm );
    return fd_to_read;
}

int send_fd( int fd, int fd_to_send )
{
    struct iovec iov[1];
    struct msghdr msg;
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    cmsghdr cm;
    cm.cmsg_len = CONTROL_LEN;
    cm.cmsg_level = SOL_SOCKET;
    cm.cmsg_type = SCM_RIGHTS;
    *(int *)CMSG_DATA( &cm ) = fd_to_send;
    msg.msg_control = &cm; /* 设置辅助数据 */
    msg.msg_controllen = CONTROL_LEN;

    /* 这是一个专门用来传递 fd 的系统调用 */
    sendmsg( fd, &msg, 0 );
}


/* 读不出来啊?为啥 */
int main()
{
    int pipefd[2];
    int fd_to_pass = 0;
    int ret = socketpair( PF_UNIX, SOCK_STREAM, 0, pipefd );
    assert( ret != -1 );

    pid_t pid = fork();
    assert( pid >= 0 );
    if( pid == 0 )
    {
        close( pipefd[0] );
        fd_to_pass = open( "test.txt", O_RDWR, 0666 );
        // cout << "true fd " << fd_to_pass << endl;
        /* 子进程通过管道将 fd 发送给父进程.若文件打开失败,发送标准输入文件符 */
        send_fd( pipefd[1], (fd_to_pass > 0 ) ? fd_to_pass : 0 );
        close( fd_to_pass );
        exit( 0 );
    }
    close( pipefd[1] );
    fd_to_pass = recv_fd( pipefd[0] );
    char buf[1024];
    memset( buf, '\0', 1024 );
    cout << "get fd: " << fd_to_pass << endl;
    read( fd_to_pass, buf, 1024 );
    cout << " get message: " << buf << endl;
    close( fd_to_pass );    
}

/* 直接用 send recv 打不开,为什么? */
int main2()
{
    int pipefd[2];
    int fd_to_pass = 0;
    int ret = socketpair( PF_UNIX, SOCK_STREAM, 0, pipefd );
    assert( ret != -1 );

    pid_t pid = fork();
    assert( pid >= 0 );
    char fd;
    if( pid == 0 )
    {
        close( pipefd[0] );
        fd_to_pass = open( "test.txt", O_RDWR, 0666 );
        cout << "true fd " << fd_to_pass << endl;
        fd = fd_to_pass + 48;       
        /* 子进程通过管道将 fd 发送给父进程.若文件打开失败,发送标准输入文件符 */
        send( pipefd[1], &fd, sizeof(fd), 0 );
        close( fd_to_pass );
        exit( 0 );
    }
    close( pipefd[1] );
    char get[10];
    recv( pipefd[0], &get, sizeof(fd), 0 );
    
    char buf[1024];
    memset( buf, '\0', 1024 );
    int get_fd = *get - 48;
    cout << "get fd: " << get_fd << endl;
    int res = read( get_fd, buf, 1024 );
    cout << " get message: " << buf << endl;
    close( fd_to_pass );    
}