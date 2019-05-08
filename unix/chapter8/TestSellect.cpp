#include "header.h"
using namespace std;

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

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
    if(connfd < 0){
        cout << "error:" << errno << endl;
    }else
    {
        char buf[1024];
        // fdset为一个数组,每一位标记一个文件描述符
        fd_set read_fds;
        fd_set exception_fds;
        // 每位清零
        FD_ZERO(&read_fds);
        FD_ZERO(&exception_fds);

        while(1){
            memset(buf, '\0', sizeof(buf));
            // 每次调用select前都要重新设置文件描述符connfd,因为事件发生之后,
            // 文件描述符集合会被内核修改
            FD_SET(connfd, &read_fds);
            FD_SET(connfd, &exception_fds);
            // nfds:被监听的文件描述符的总数,通常被设置Wieselect监听的所有文件描述符
            // 中的最大值+1
            ret = select(connfd + 1, &read_fds, NULL, &exception_fds, NULL);
            if(ret <= 0){
                cout << "error" << errno << endl;
                break;
            }
            // 可读事件,采用普通的recv读取数据
            if(FD_ISSET(connfd, &read_fds)){
                ret = recv(connfd, buf, sizeof(buf)-1, 0);
                if(ret <= 0){
                    break;
                }
                cout << "nomal data :" << buf << endl;
                // 异常事件,用MSG_OOB 的 recv 读取数据
            }else if (FD_ISSET(connfd, &exception_fds))
            {
                ret = recv(connfd, buf, sizeof(buf) - 1, MSG_OOB);
                if(ret <= 0){
                    break;
                }
                cout << "OOG data :" << buf << endl;
            }
        }
    }
    close(connfd);
    close(listenfd);
    return 0;
}