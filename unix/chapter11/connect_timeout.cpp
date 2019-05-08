#include "header.h"
using namespace std;

/**
 * ＳＯ＿ＲＣＶＴＩＭＥＯ　和　ＳＯ＿ＳＮＤＴＩＭＥＯ：
 * 用来设置ｓｏｃｋｅｔ接收数据超时间和发送数据超时时间．仅对数据接受和发送相关的ｓｏｃｋｅｔ专用系统调用有效．
 *　以ｃｏｎｎｅｃｔ为例子，使用ＳＯ＿ＳＮＤＴＩＭＥＯ来定时
 */

int timeout_connect(const char* ip, int port, int time)
{
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    /*通过选项SOCK_...和scok...所设置的超时时间的类型为timeval,这和select系统调用
    的超时参数类型相同*/
    struct timeval timeout;
    socklen_t len = sizeof(timeout);
    timeout.tv_sec = time;
     timeout.tv_usec = 0;
     // 设置接收数据超时时间
     ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
     assert(ret != -1);

     ret = connect(sockfd, (struct sockaddr*)&address, sizeof(address));
     if(ret == -1)
     {
         // 超时对应的错误号是EINPROGRESS，若下列条件成立，则可以处理定时任务
         if(errno == EINPROGRESS)
         {
             cout << "connetion timeout,do something" << endl;
         }
         cout << "error occur" << endl;
         return -1;
     }
     return sockfd;
}

int main(int argc, char* argv[])
{
    if(argc <= 2)
    {
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = timeout_connect(ip, port, 10);
    if(sockfd < 0)
    {
        return 1;
    }
    cout << "normal" << endl;
    return 0;
}