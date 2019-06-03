#include "header.h"
using namespace std;
/**
 * splice:用于两个文件描述符之间移动数据(零拷贝)
 * 使用slipce实现一个回射服务器,把客户端发送的数据返回给客户端
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
    // 把IP地址转为网络字节序表示
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
    int connfd = accept(sock, (struct sockaddr*)&client, &client_addrlength);
    if(connfd < 0){
        cout << "error:" << errno << endl;
    }else
    {
        int pipefd[2];
        // 创建管道
        ret = pipe(pipefd);
        assert(ret != -1);
        // 把connfd上流入的客户端数据定向到管道中 
        ret = splice(connfd, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        assert(ret != -1);
        // 把管道的输出定向到connfd客户连接文件描述符
        ret = splice(pipefd[0], NULL, connfd, NULL, 32786, SPLICE_F_MORE | SPLICE_F_MOVE);
        assert(ret != -1);
        close(connfd);
    }
    close(sock);
    return 0;
}