#include "header.h"
using namespace std;
int const BUFFER_SIZE = 4096;

/**
 * 状态驱动解析HTTP请求
 */

/**
 * 主状态机的两种状态,请求行和请求头
 */
enum CHECK_STATE{CHECK_STATE_REQUESTLINE, CHECK_STATE_HEADER};

/**
 * 从状态机的状态,行的读取状态:读取到完整行\出错\数据不完整
 */
enum LINE_STATUS{LINE_OK, LINE_BAD, LINE_OPEN};

/**
 * http返回码
 */
enum HTTP_CODE{NO_REQUEST, GET_REQUEST, BAD_REQUEST, FORBIDDEN_REQUEST,
 INTERNAL_ERROR, CLOSED_CONNNECTION};
 
 static const char* szret[] = {"I get a correct result\n", "something wrong\n"};

/**
 * 解析请求的每一行
 * check_index:指向当前正在分析的字节
 * read_index:指向bufer中客户数据的尾部的下一字节
 */
 LINE_STATUS parse_line(char* buffer, int& check_index, int& read_index){
     char temp;
     for(;check_index < read_index;++check_index){
         // 获得当前要分析的字节
         temp = buffer[check_index];
         if(temp == '\r'){
             // 若'\r'是最后一个字符,则需要进一步获取
             if((check_index + 1) == read_index){
                 return LINE_OPEN;
                 // 读取到完整的一行
             }else if (buffer[check_index + 1] == '\n')
             {
                 buffer[check_index++] = '\0';
                 buffer[check_index++] = '\0';
                 return LINE_OK; 
             }
             return LINE_BAD;
         }else if (temp == '\n')
         {
             // 读取到完整的一行
             if((check_index > 1) && (buffer[check_index - 1]) == '\r'){
                 buffer[check_index - 1] = '\0';
                 buffer[check_index++] = '\0';
                 return LINE_OK;
             }
             return LINE_BAD;
         }
    }
    // 所有内容解析完都没'\r',说明要继续读取
    return LINE_OPEN;
 }


 HTTP_CODE parse_reqeustline(char* temp, CHECK_STATE& checkstate)
 {
     // 找到第一个出现 \t(跳格)的位置
     char* url = strpbrk(temp, " \t");
     if(!url){
         return BAD_REQUEST;
     }
     *url++ = '\0';
     char* method = temp;
     // 忽略大小写比较是否有get,如果有返回0
     if(strcasecmp(method, "GET") == 0){
         cout << "the method is get" << endl;
     }else
     {
         return BAD_REQUEST;
     }
     // strspn:找到str1中第一个不在str2中出现的字符下标
     url += strspn(url, " \t");
     char* version = strpbrk(url, " \t");
     if(!version){
         return BAD_REQUEST;
     }
     *version++ = '\0';
     version += strspn(version, " \t");
     // 仅支持http1.1
     if(strcasecmp(version, "HTTP/1.1") == 0){
         return BAD_REQUEST;
     }
     if(strncasecmp(url, "http://", 7) == 0){
         url += 7;
         url = strchr(url, '/');
     }
     if(!url || url[0] != '/'){
         return BAD_REQUEST;
     }
     cout << "request url is " << url << endl;
     checkstate = CHECK_STATE_HEADER;
     return NO_REQUEST;
 }

 HTTP_CODE parse_header(char* temp){
     // 遇到一个空行,说明得到了一个正确的http请求
     if(temp[0] == '\0'){
         return GET_REQUEST;
     }else if (strncasecmp(temp, "Host:", 5) == 0)
     {
         temp += 5;
         temp += strspn(temp, " \t");
         cout << "the request host is " << temp << endl;
     }else
     {
         cout << "cnan ot handler this header" << endl;
     }
     
 }

/**
 * 解析HTTP请求入口函数
 */
 HTTP_CODE parse_content(char* buffer, int& check_index,CHECK_STATE &checkstate, 
 int& read_index, int& start_line){
     LINE_STATUS linestatus = LINE_OK;
     HTTP_CODE retcode = NO_REQUEST;
     while((linestatus = parse_line(buffer, check_index, read_index)) == LINE_OK){
      // start_line 为行在buffer中的起始位置
     char* temp = buffer + start_line;
     start_line = check_index;
     switch (checkstate)
     {
         case CHECK_STATE_REQUESTLINE: // 分析请求行
             retcode = parse_reqeustline(temp, checkstate);
             if(retcode == BAD_REQUEST){
                 return BAD_REQUEST;
             }
             break;
         case CHECK_STATE_HEADER:
             retcode = parse_header(temp);
             if(retcode == BAD_REQUEST){
                 return BAD_REQUEST;
             }else if (retcode == GET_REQUEST)
             { 
                 return GET_REQUEST;
             }
             break;
         default:
             return INTERNAL_ERROR;
     }        
     }
     if(linestatus == LINE_OPEN){
         return NO_REQUEST;
     }else
     {
         return BAD_REQUEST;
     }
 }

 /**
  * 主函数入口
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
        cout << "error :" << errno << endl;
    }else
    {
        char buffer[BUFFER_SIZE];
        memset(buffer, '\0', BUFFER_SIZE);
        int data_read = 0;
        int read_index = 0;
        int checked_index = 0;
        int start_line = 0;
        CHECK_STATE checkstate = CHECK_STATE_REQUESTLINE;
        while(1){ // 循环读取客户数据分析
            data_read = recv(connfd, buffer + read_index, BUFFER_SIZE - read_index, 0);
            if(data_read == -1){
                cout << "read failed" << endl;
                break;
            }else if (data_read == 0)
            {
                cout << "remote client has closed the connection" << endl;
                break;
            }
            read_index += data_read;
            HTTP_CODE result = parse_content(buffer, checked_index, checkstate, 
            read_index, start_line);
            if(result == NO_REQUEST){
                continue;
            }else if (result == GET_REQUEST)
            {
                send(connfd, szret[0], strlen(szret[0]), 0);
                break;
            }else // 其他情况表示发生错误
            {
                send(connfd, szret[1], strlen(szret[0]), 0);
                break;
            }
        }
        close(connfd);
    }
     close(listenfd);
     return 0;
 }