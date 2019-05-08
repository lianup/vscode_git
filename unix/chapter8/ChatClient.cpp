#include "header.h"

using namespace std;

const int BUFFER_SIZE = 64;

/**
 * chatclient:1.从标准输入终端读入用户数据,把用户数据发送给服务器;
 * 2.往标准输出终端打印服务器发送给它的数据
 */
/**
 * 使用poll监听用户输入和网络连接,利用splice函数把用户输入的内容直接定向到网络上
 */
int main(int argc, char* argv[])
{
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in server;

    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server.sin_addr);
    server.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    
    if(connect(sockfd, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        cout << "connection failed" << endl;
        close(sockfd);
        return 1;
    }
    // 注册文件描述符0(标准输入)和文件描述符sock上的可读事件
    pollfd fds[2];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = sockfd;
    // pollrdhup:TCP连接被对方关闭,或者对方关闭了写操作,它由GNU引入
    fds[1].events = POLLIN | POLLRDHUP;
    fds[1].revents = 0;
    
    char buf[BUFFER_SIZE];
    int pipefd[2];
    int ret = pipe(pipefd);
    assert(ret != -1);

// 把socket传来的消息打印出来,把标准输入的消息发给sock
    while(1)
    {
        ret = poll(fds, 2, -1);
        if(ret < 0){
            cout << "poll failed" << endl;
            break;
        }
        // socket上有可读数据(来自服务器传输的其他客户端的数据),输出
        if(fds[1].revents & POLLIN){
            memset(buf, '\0', BUFFER_SIZE);
            recv(fds[1].fd, buf, BUFFER_SIZE - 1, 0);
            // ret = recv(sockfd, buf, BUFSIZ - 1, 0);
            cout << buf << endl;
        }
        else if (fds[1].revents & POLLRDHUP)
        {
            cout << "server closed the connection" << endl;
            break;
        }

        if (fds[0].revents & POLLIN)        
        // else if (fds[0].revents & POLLIN)
        {
            // memset(buf, '\0', BUFSIZ - 1);
            // 使用此函数时,in和out至少有一个是管道fd,标准输入的数据读到管道1
            ret = splice(0, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
            // 把数据从管道0到送到sockfd
            ret = splice(pipefd[0], NULL, sockfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        }
    }
    close(sockfd);
    return 0;
}