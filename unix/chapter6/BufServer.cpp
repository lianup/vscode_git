#include "header.h"
using namespace std;
const int BUF_SIZE(1024);

/**
 * 设置TCP接收窗口大小
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
    int sock = socket(PF_INET,SOCK_STREAM, 0);
    assert(sock >= 0);
    int recvbuf = atoi(argv[3]);
    int len = sizeof(recvbuf);
    // 设置TCP接受缓冲区的大小,然后立即读取
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &recvbuf, sizeof(recvbuf));
    getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &recvbuf, (socklen_t*)&len);
    cout << "the tcp receive buffer size after seeting is" << recvbuf << endl;
    
    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    
    ret = listen(sock, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock, (struct sockaddr*)&client, &client_addrlength);
    if(connfd < 0){
        cout << "error is :" << errno << endl;
    }else
    {
        char buffer[BUF_SIZE];
        memset(buffer, '\0', BUF_SIZE);
        while(recv(connfd, buffer, BUF_SIZE-1, 0) > 0){}
        close(connfd);
    }
    close(sock);
    return 0;
}