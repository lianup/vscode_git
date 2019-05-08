#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <string.h>

static bool stop = false;
/**
 * sigtem信号的处理函数,触发时结束主程序中的循环
 */
static void handle_term(int sig){

    stop = true;
}

/**
 * 监听 socket 程序
 */
int main(int argc, char* argv[]){

    signal( SIGTERM, handle_term);
    if(argc <= 3){
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    // 用来将点分十进制字符串表示的IPV4地址转化为用网络字节序整数表示的IPV4地址
    int backlog = atoi(argv[3]);
    // 创建socket,domain-PF_INET 表IPV4,type-服务类型-流/数据报服务
    // protocol-协议版本-0默认
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    // 创建成功返回一个socket文件描述符
    assert(sock >= 0);
    // 创建一个IPv4的socket地址
    struct sockaddr_in address;
    bzero( &address, sizeof(address));
    //地址族:IPV4协议
    address.sin_family = AF_INET;
    // 将字符串表示的IP地址src转换成用网络字节序整数表示的IP地址
    inet_pton(AF_INET, ip, &address.sin_addr);
    // host to network short(把短整形的主机字节序数据(端口号)转换成网络字节序数据)
    address.sin_port = htons(port);
    // 把address的socket地址分配给未命名的sockfd描述符,addrlen指出该socket地址的长度
    int ret = bind(sock, (struct sockaddr*) &address, sizeof(address));
    assert(ret != -1);
    // 创建一个监听队列来存放待处理的客户连接
    ret = listen(sock, backlog);
    assert(ret != -1);
    while(!stop){
        sleep(1);
    }
    // 关闭连接
    close(sock);
    return 0;
}

