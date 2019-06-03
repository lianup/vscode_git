#include "header.h"
using namespace std;


/**
 * 此种方法存在移植性问题
 * 1.非阻塞的socket可能导致连接始终失败
 * 2.select对处于EINPROGRESS状态下的socket可能不起作用
 * 3.对出错的socket,getsockopt在有些系统上返回-1,有些返回0
 */
/**
 * 实现非阻塞连接
 * 对非阻塞的socket调用connet,连接有没有立即建立时会返回EINPROGRESS错误;
 * 此时可以调用select\poll等函数来监听此socket上的写事件,当返回时,利用etsockopt来读取
 * 错误码并清楚该socket上的错误即可.若错误码为0,表连接成功建立,否则为失败.
 */

const int BUFFER_SIZE = 1204;

int setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

/**
 * 超时连接函数,函数成功时返回已经处于连接状态的socket,否则返回-1
 */
int unblock_connect(const char* ip, int port, int time){
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    int fdopt = setnonblocking(sockfd);
    ret = connect(sockfd, (struct sockaddr*)&address, sizeof(address));
    // 连接成功
    if(ret == 0){
        cout << "connet server imediately" << endl;
        // 恢复sockfd的属性
        fcntl(sockfd, F_SETFL, fdopt);
        return sockfd;
    }
    else if (errno != EINPROGRESS)
    {  
        // 其他错误出错返回
        cout << "other error : " << errno << endl;
        return -1;
        
    }
    fd_set readfds;
    fd_set writefds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &writefds);
    struct timeval timeout;
    timeout.tv_sec = time;
    timeout.tv_usec = 0;

    ret = select(sockfd + 1, NULL, &writefds, NULL, &timeout);
    if(ret <= 0){
        // select 出错,立即返回
        cout << "select error : " << errno << endl;
        return -1;
    }
    // 没有出错,但是此fd没有写事件发生
    if(!FD_ISSET(sockfd, &writefds)){
        cout << "no events on sockfd " << endl;
        close(sockfd);
        return -1;
    }
    int error = 0;
    socklen_t length = sizeof(error);
    
    // 调用getsockopt来获取并清除sockfd上的错误
    if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &length) < 0){
        cout << "get socket option error" << endl;
        close(sockfd);
        return -1;
    }
    // 错误号不为0表示连接出错
    if(errno != 0){
        cout << "connetion faild after select with the error " << endl;
        close(sockfd);
        return -1;
    }
    // 错误号为0,连接成功
    cout << "coonection ready after select with the socket" << sockfd << endl;
    fcntl(sockfd, F_SETFL, fdopt);
    close(sockfd);
    return 0;
}

int main(int argc, char* argv[]){
    if(argc <= 2){
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int sockfd = unblock_connect(ip, port, 10);
    if(sockfd < 0)
    {
        return 1;
    }
    close(sockfd);
    return 0;
}
