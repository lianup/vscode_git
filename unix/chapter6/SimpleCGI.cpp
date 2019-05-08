#include "header.h"
using namespace std;

/**
 * dup:创建一个新的文件描述符,与原来的fd指向相同的文件\管道\socket
 * dup2:与dup类似,但是它返回第一个不小于file_descriptor_two的整数值
 * 使用dup函数实现一个基本的CGI服务器
 * CGI服务器:使得程序具有交互功能
 */
int main(int argc, char* argv[]){

    if(argc <= 2){
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);
    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(sock, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock, (struct sockaddr*)&client,&client_addrlength);
    if(connfd < 0){
        cout << "error :" << errno << endl;
    }else
    {
        // 关闭标准输出文件描述符(值为1)
        close(STDOUT_FILENO);
        // 复制socket文件描述符,dup总是返回系统中可用的最小fd,故返回1
        dup(connfd);
        // 这样,服务器的标准输出的内容,就会直接发送到与客户端连接的socket上
        printf("abcd\n");
        close(connfd);
    }
    close(sock);
    return 0;
}