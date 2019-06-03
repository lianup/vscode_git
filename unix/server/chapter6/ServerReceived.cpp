#include "header.h"
using namespace std;

/**
 * 接受消息的服务端
 */
const int BUF_SIZE(1024);
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
    int sock = socket(PF_INET,SOCK_STREAM, 0);
    assert(sock >= 0);
    // 命名socket,只有命名后客户端才知道如何连接它,将address所指的socket地址分配给
    // 未命名的sock
    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    
    // socket被命名后,需要创建监听队列来存放待处理的客户连接
    ret = listen(sock, 5);
    assert(ret != -1);
    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    // 从listen监听队列中接受连接
    int connfd = accept(sock, (struct sockaddr*)&client, &client_addrlength);
    if(connfd < 0){
        cout << "error :" << errno; 
    }else
    {
        char buffer[BUF_SIZE];
        memset(buffer, '\0', BUF_SIZE);
        // 接收消息
        ret = recv(connfd, buffer, BUF_SIZE - 1, 0);
        cout << "got normal data :" << buffer << endl;

        memset(buffer, '\0', BUF_SIZE);
        ret = recv(connfd, buffer, BUF_SIZE - 1, MSG_OOB);
        cout << "got oob data :" << buffer << endl;   

        
        memset(buffer, '\0', BUF_SIZE);
        ret = recv(connfd, buffer, BUF_SIZE - 1, 0);
        cout << "got normal data :" << buffer << endl;      
    }
    
    close(ret);    
    return 0;
}