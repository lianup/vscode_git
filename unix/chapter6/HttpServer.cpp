#include "header.h"
#include <sys/uio.h>
using namespace std;
/**
 * web服务器接受到文件请求后,返回文件
 */
const int BUF_SIZE(1024);
static const char* status_line[2] = {"200 OK", "500 Internal server error"};

int main(int argc, char* argv[]){
    if(argc <= 3){
        return 1;
    }
    const char* ip = argv[1];
    // 字符串转为整数值
    int port = atoi(argv[2]);
    const char* file_name = argv[3];
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    // 转换为网络字节序的端口号
    address.sin_port = htons(port);
    
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);
    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(sock, 5);
    assert(ret != -1);
    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock, (struct sockaddr*)&client,&client_addrlength);
    if(connfd < 0){
        cout << "error:" << errno << endl;
    }else
    {
        // 用于存放HTTP应答的状态行,头部字段和一个空行的缓冲区
        char header_buf[BUF_SIZE];
        memset(header_buf, '\0', BUF_SIZE);
        // 存放目标文件内容
        char* file_buf;
        // 用于获取目标文件的属性,如是否为目录,文件大小等
        struct stat file_stat;
        bool valid = true;
        // 缓冲区header_buf目前已经使用了多少字节
        int len = 0;
        // 目标文件不存在
        if(stat(file_name,&file_stat) < 0){
            valid = false;
        }else
        {
            // 目标文件为目录
            if(S_ISDIR(file_stat.st_mode)){
                valid = false;
                // 如果目标文件不为目录且当前用户有权读取
            }else if(file_stat.st_mode && S_IROTH)
            {
                int fd = open(file_name, O_RDONLY);
                file_buf = new char[file_stat.st_size+1];
                memset(file_buf, '\0', file_stat.st_size + 1);
                if(read(fd, file_buf, file_stat.st_size) < 0){
                    valid = false;
                }
            }else
            {
                valid = false;
            }
        }
        // 若文件有效,则发送正常的HTTP应答
        if(valid){
            ret = snprintf(header_buf, BUF_SIZE -1 ,"%s %s\r\n", "HTTP/1.1"
            , status_line[0]);
            len += ret;
            ret = snprintf(header_buf + len, BUF_SIZE - 1 - len, 
            "Content-Length: %d\r\n", file_stat.st_size);
            len += ret;
            ret = snprintf(header_buf + len, BUF_SIZE - 1 - len, "%s", "\r\n");

            // 利用writev把header_buf和file_buf的内容一起写出
            struct iovec iv[2];
            iv[0].iov_base = header_buf;
            iv[0].iov_len = strlen(header_buf);
            iv[1].iov_base = file_buf;
            iv[1].iov_len = file_stat.st_size;
            ret = writev(connfd, iv, 2);
        }else
        {
            ret = snprintf(header_buf, BUF_SIZE - 1, "%s %s\r\n",
             "HTTP/1.1", status_line[1]);
             len += ret;
             ret = snprintf(header_buf + len, BUF_SIZE - 1 - len, "%s", "\r\n");
             send(connfd, header_buf, strlen(header_buf), 0);
        }
        close(connfd);
        delete[] file_buf;
    }
    return 0;
}