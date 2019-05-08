#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <string.h>
using namespace std;
/**
 * 接受 socket 连接程序
 */
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
    int sock = socket( PF_INET, SOCK_STREAM, 0);
    assert(sock > 0);
    // 把address的socket地址分配给未命名的sockfd描述符,addrlen指出该socket地址的长度
    int ret = bind(sock, (struct sockaddr*) &address, sizeof(address));
    assert(ret != -1);
    // 创建一个监听队列来存放待处理的客户连接
    ret = listen(sock, 5);
    assert(ret != -1);
    sleep(20);
    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    // accept 只是从监听队列中取出链接,而不管连接处于何种状态
    int connfd = accept(sock, (struct sockaddr*)&client, &client_addrlength);
    if(connfd < 0){
        std::cout << "error is :" << errno;
    }else
    {
        // 接收成功则打印出客户端的IP地址和端口号
        char remote[INET_ADDRSTRLEN];
        cout << "connect with ip: " << inet_ntop(AF_INET, &client.sin_addr, remote, INET_ADDRSTRLEN)
             << "port: " << ntohs(client.sin_port) << endl; 
    }
    // 关闭链接,并非总是立即关闭一个连接,而是把fd的引用计数-1,立即终止可用shutdown
    close(sock);
    return 0;
}

