#include "header.h"
using namespace std;

/**
 * 使用sendfile(零拷贝)发送文件
 */
int main(int argc, char* argv[]){
    if(argc <= 3){
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    const char* file_name = argv[3];
    
    int filefd = open(file_name, O_RDONLY);
    assert(filefd > 0);
    struct stat stat_buf;
    fstat(filefd, &stat_buf);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock, (struct sockaddr*) &address, sizeof(address));
    assert(ret != -1);
    ret = listen(sock, 5);
    assert(ret != -1);
    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock, (struct sockaddr*) &client, &client_addrlength);
    if(connfd < 0){
        cout << "error: " << errno << endl;
    }else
    {
        // out_fd:只能是socket; in_fd:只能是文件;offset:偏移量;
        // count : 传输的字节数
        sendfile(connfd, filefd, NULL, stat_buf.st_size);
    }
    close(sock);
    return 0;
}