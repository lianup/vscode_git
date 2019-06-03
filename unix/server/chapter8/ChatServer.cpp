#include "header.h"
using namespace std;
const int USER_LIMIT = 5;
const int BUFFER_SIZE = 64;
const int FD_LIMITE = 65535;


/**
 * 接受客户数据,并把客户数据发送给每一个登录到该服务器上的客户端(数据发送者除外)
 */


/**
 * 客户信息:socket地址\待写到客户端的数据的位置\从客户端读入的数据
 */
struct client_data{
    sockaddr_in address;
    char* write_buf;
    char buf[BUFFER_SIZE];
};

int setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, new_option);
    return old_option;
}


/**
 * 服务器使用poll同时管理监听socket和连接socket
 */
int main(int argc,char* argv[]){
    if(argc <= 2){
        return 1;
    }
    const char* ip =argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET,SOCK_STREAM, 0);
    assert(listen >= 0);

    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    // 创建user数组,分配FD_LIMIT个client_data对象:每个socket都可以获得一个对象,并且
    // socket的值可以用来直接索引socket链接对应的client_data
    client_data* users = new client_data[FD_LIMITE]; 
    pollfd fds[USER_LIMIT + 1];
    int user_counter = 0;
    for(int i = 1;i < USER_LIMIT;++i){    
    // for(int i = 0;i < USER_LIMIT;++i){
        fds[i].fd = -1;
        fds[i].events = 0;
    }
    // fds[0]用来监听链接
    fds[0].fd = listenfd;
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;

    while(1)
    {
        ret = poll(fds, user_counter + 1, -1);
        // ret = poll(fds, user_counter, -1);
        if(ret < 0){
            cout << "poll failure" << endl;
            break;
        }
        // 轮询是否有fd准备好
        for(int i = 0;i < user_counter+1;++i){
            // 如果fd[0](监听连接的fd)有数据准备好读,则说明要创建新连接
            if((fds[i].fd == listenfd) && (fds[i].revents & POLLIN))
            // if((fds[i].fd == listenfd) && (fds[i].revents == POLLIN))
            {
                // 创建客户连接
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength
                );
                if(connfd < 0)
                {
                    cout << "connect faild" << endl;
                    continue;
                }
                // 如果请求太多,则关闭链接
                if(user_counter >= USER_LIMIT)
                {
                    const char* info = "too many users";
                    cout << info << endl;
                    send(connfd, info, strlen(info), 0);
                    close(connfd);
                    continue;
                }
                // 否则添加新用户,同时修改fds和users数组
                ++user_counter;
                users[connfd].address = client_address;
                setnonblocking(connfd);
                fds[user_counter].fd = connfd;
                fds[user_counter].events = POLLIN | POLLERR | POLLRDHUP;
                fds[user_counter].revents = 0;
                cout << "comes a new user : " << user_counter << endl;
            }
            else if (fds[i].revents & POLLERR)
            {
                cout << "get an error from " << user_counter << endl;
                char errors[100];
                memset(errors, '\0', sizeof(errors));
                socklen_t length = sizeof(errors);
                if(getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &length) < 0)
                {
                    cout << "get socket option faild" << endl;
                }
                continue;
            }
            // 客户端关闭了链接
            else if (fds[i].revents & POLLRDHUP)
            {
                users[fds[i].fd] = users[fds[user_counter].fd];
                close(fds[i].fd);
                fds[i] = fds[user_counter];
                --i;
                --user_counter;
                cout << "a client left" << endl;
            
            }
            // 客户端发送了消息
            else if (fds[i].revents & POLLIN)
            {
                int connfd = fds[i].fd;
                memset(users[connfd].buf, '\0', BUFFER_SIZE);
                ret = recv(connfd, users[connfd].buf, BUFFER_SIZE - 1, 0);
                cout << "get client data from " << connfd << ":" << users[connfd].buf << endl;
                if(ret < 0){
                    // 读操作出错,则关闭链接
                    if(errno != EAGAIN)
                    {
                        close(connfd);
                        users[fds[i].fd] = users[fds[user_counter].fd];
                        fds[i] = fds[user_counter];
                        --i;
                        --user_counter; 
                    }
                }
                else if (ret == 0)
                {
                    
                }
                else
                {
                    // 接受正常,则通知其他客户端写数据
                    for(int j = 1;j <= user_counter; ++j){
                        if(fds[j].fd == connfd){
                            continue;
                        }
                        fds[j].events |= ~POLLIN;
                        fds[j].events |= POLLOUT;
                        users[fds[j].fd].write_buf = users[connfd].buf;
                    }
                }  
            }
            //  如果客户端写就绪,那就向其写入数据
            else if (fds[i].revents & POLLOUT)
            {
                int connfd = fds[i].fd;
                if(!users[connfd].write_buf){
                    continue;
                }
                ret = send(connfd, users[connfd].write_buf, strlen(users[connfd].write_buf), 0);
                users[connfd].write_buf = NULL;
                // 写完数据后需要重新注册fds[i]上的可读事件
                fds[i].events = ~POLLOUT;
                fds[i].events = POLLIN;
            }
        }
    }
    delete[] users;
    close(listenfd);
    return 0;
}