#include "header.h"
using namespace std;

int main(int argc, char* argv[]){

    if(argc <= 2){
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    // Tcp协议专有的socket地址结构体
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    // 端口转换
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    server_address.sin_port = htons(port);
    // 建立socket链接
    int sockfd = socket(PF_INET,SOCK_STREAM, 0);
    assert(sockfd >= 0);
    // 客户端与服务器建立链接,成功返回0
    if(connect(sockfd,(struct sockaddr*)&server_address, sizeof(server_address)) < 0){
        cout << "connection failed" << endl;
    }else
    {
        const char* oob_data = "hello";
        const char* normal_data = "i am client";
        // 发送数据
        send(sockfd, normal_data, strlen(normal_data), 0);
        // oob紧急数据
        send(sockfd, oob_data, strlen(oob_data), MSG_OOB);
        send(sockfd, normal_data, strlen(normal_data), 0);
    }
    close(sockfd);    
    return 0;
}