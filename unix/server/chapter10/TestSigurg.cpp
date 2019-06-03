#include "header.h"
using namespace std;
const int BUF_SIZE = 1024;

static int connfd;

/*SIGUG 信号的处理函数*/
void sig_urg(int sig)
{
    int save_error = errno;
    char buffer[BUF_SIZE];
    // 接收带外数据
    int ret = recv(connfd, buffer, BUF_SIZE, MSG_OOB);
    cout << "got data :" << buffer << endl;
    // 恢复原来的错误码
    errno = save_error;
}

void addsig(int sig, void (*sig_handler)(int))
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    // sa.__sigaction_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int main(int argc, char* argv[])
{
    if(argc <= 2){
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address);
    address.sin_port = htons(port);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock > 0);

    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    connfd = accept(sock, (struct sockaddr*)&client, &client_addrlength);
    if(connfd < 0)
    {
        cout << "error is :" << errno << endl;
    }
    else
    {
        addsig(SIGURG, sig_urg);
        //　使用ＳＩＧＵＲＧ信号之前，设置ｓｏｃｋｅｔ的宿主进程ｏｒ进程组
        fcntl(connfd, F_SETOWN, getpid());
        char buffer[BUF_SIZE];
        // 循环接受普通数据
        while(1)
        {
            memset(buffer, '\0', BUF_SIZE);
            ret = recv(connfd, buffer, BUF_SIZE - 1, 0);
            if(ret <= 0)
            {
                break;
            }
            cout << "get normal data :" << buffer << endl; 
        }
            close(connfd);        
           
        }
    close(sock);
    return 0;
}