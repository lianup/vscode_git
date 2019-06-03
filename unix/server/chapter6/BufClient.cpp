#include "header.h"
using namespace std;
const int BUF_SIZE(1024);

/**
 * 设置TCP发送窗口大小
 */
int main(int argc, char* argv[]){

    if(argc <= 2){
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    server_address.sin_port = htons(port);
    int sock = socket(PF_INET,SOCK_STREAM, 0);
    assert(sock >= 0);
    int sendbuf = atoi(argv[3]);
    int len = sizeof(sendbuf);
    // 设置TCP发送缓冲区的大小,然后立即读取
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf, sizeof(sendbuf));
    getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf, (socklen_t*)&len);
    cout << "the tcp send buffer size after seeting is" << sendbuf << endl;
    
    if(connect(sock, (struct sockaddr*)&server_address, sizeof(server_address) != -1)){
        char buffer[BUF_SIZE];
        memset(buffer, 'a', BUF_SIZE);
        send(sock, buffer,BUF_SIZE,0);
    }
    close(sock);
    return 0;
}