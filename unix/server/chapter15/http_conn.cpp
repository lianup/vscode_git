#include "http_conn.h"
using std::cout;
using std::endl;

/* HTTP 响应的一些信息 */
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file from this server";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server";
const char* error_500_title = "Internal Error";
const char* error_500_form = "THere was an unusual problem serving the requested file";

/* 网站根目录 */
const char* doc_root = "/var/html";
int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    /* int new_option = fd | O_NONBLOCK;*/
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}


/* 因为为多线程异步,为了防止同一个 socket 的同一种事件被多个线程处理,引入 oneshot
   设置 ONESHOT ,则某 socket 上的某事件只会被触发一次,要重新出发得重新注册事件 */
void addfd( int epollfd, int sockfd, bool one_shot )
{
    epoll_event event;
    event.data.fd = sockfd;
    /* 设置为可读事件, LT 模式, socket 断开连接事件 */
    /* event.events = EPOLLIN | EPOLLET | EPOLLHUP; */
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    /* 这个操作得在后面 */
    /* epoll_ctl( epollfd, EPOLL_CTL_ADD, sockfd, &event ); */
    if( one_shot )
    {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, sockfd, &event );
    setnonblocking( sockfd );
}

void removefd( int epollfd, int sockfd ) 
{
    epoll_ctl( epollfd, EPOLL_CTL_DEL, sockfd, 0 );
    close( sockfd );
}

/* 修改 fd 上面的监听事件类型 */
void modfd( int epollfd, int sockfd, int ev )
{
    epoll_event event;
    event.data.fd = sockfd;
    event.events = ev | EPOLLET |  EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl( epollfd, EPOLL_CTL_MOD, sockfd, &event );
}

int http_conn::m_user_count = 0;
int http_conn::m_epollfd = -1;

/* 把线程池中的状态量都初始化 */
void http_conn::init()
{
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;

    /* 仅支持 GET */
    m_method = GET;
    m_url = 0;
    m_version = 0;
    m_content_length = 0;
    m_host = 0;
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
    memset( m_read_buf, '\0', READ_BUFFER_SIZE );
    memset( m_write_buf, '\0', WRITE_BUFFER_SIZE );
    memset( m_real_file, '\0', FILE_NAME_LEN );
}

/* 初始化 socket 连接 */
void http_conn::init( int sockfd, const sockaddr_in& addr )
{
    m_sockfd = sockfd;
    m_address = addr;
    /* 下面两行是Wie了避免 TIME_WAIT 状态,仅用于调试,实际用时去掉 */
    // int reuse = 1;
    // setsockopt( m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( reuse ) );
    addfd( m_epollfd, sockfd, true );
    ++m_user_count;
    init();
}

/* 关闭连接,一个 客户 对应 一个 socket */
void http_conn::close_conn( bool real_close )
{
    if( real_close && (m_sockfd != -1) )
    {
        removefd( m_epollfd, m_sockfd );
        m_sockfd = -1;
        --m_user_count;
    }
}

/**
 * 解析请求的每一行
 * check_index:指向当前正在分析的字节
 * read_index:指向bufer中客户数据的尾部的下一字节
 */
 http_conn::LINE_STATUS http_conn::parse_line()
 {
     char temp;
     for( ; m_checked_idx < m_read_idx; ++m_checked_idx )
     {
         // 获得当前要分析的字节
         temp = m_read_buf[ m_checked_idx ];
         if( temp == '\r' )
         {
             // 若'\r'是最后一个字符,则需要进一步获取
             if( (  m_checked_idx + 1 ) == m_read_idx )
             {
                 return LINE_OPEN;
                 // 读取到完整的一行
             }
             else if ( m_read_buf[ m_checked_idx + 1 ] == '\n' )
             {
                 m_read_buf[ m_checked_idx++ ] = '\0';
                 m_read_buf[ m_checked_idx++ ] = '\0';
                 return LINE_OK; 
             }
             return LINE_BAD;
         }
         else if ( temp == '\n' )
         {
             // 读取到完整的一行
             if( ( m_checked_idx > 1 ) && ( m_read_buf[ m_checked_idx - 1 ]  == '\r') )
             {
                 m_read_buf[ m_checked_idx - 1 ] = '\0';
                 m_read_buf[ m_checked_idx++ ] = '\0';
                 return LINE_OK;
             }
             return LINE_BAD;
         }
    }
    // 所有内容解析完都没'\r',说明要继续读取
    return LINE_OPEN;
 }

/* 循环读取客户数据,直到无数据或者对方关闭连接 */
bool http_conn::read()
{
    if( m_read_idx >= READ_BUFFER_SIZE )
    {
        return false;
    }
    int bytes_read = 0;
    while( true )
    {
        bytes_read = recv( m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0 );
        if( bytes_read == -1 )
        {
            /* 在VxWorks和Windows上，EAGAIN的名字叫做EWOULDBLOCK */
            if( errno == EAGAIN || errno == EWOULDBLOCK )
            {
                break;
            }
            return false;
        }
        else if ( bytes_read == 0 )
        {
            return false;
        }
        m_read_idx += bytes_read;
    }
    return true;
}

/* 解析 HTTP 请求行,获得请求方法,目标 URL ,版本号等 */

 http_conn::HTTP_CODE http_conn::parse_request_line( char* text )
 {
     // 找到第一个出现 \t(跳格)的位置
     m_url = strpbrk( text, " \t" );
     /* char* url = strpbrk(text, " \t"); */
     if( ! m_url ){
         return BAD_REQUEST;
     }
     *m_url++ = '\0';
     char* method = text;
     // 忽略大小写比较是否有get,如果有返回0
     if( strcasecmp( method, "GET" ) == 0 )
     {
	 cout << " get method " << endl;
         m_method = GET;
     }
     else
     {
         return BAD_REQUEST;
     }
     // strspn:找到str1中第一个不在str2中出现的字符下标
     m_url += strspn( m_url, " \t" );
     m_version = strpbrk(m_url, " \t");
     if( ! m_version )
     {
         return BAD_REQUEST;
     }
     *m_version++ = '\0';
     m_version += strspn( m_version, " \t" );
     // 仅支持http1.1
     if( strcasecmp( m_version, "HTTP/1.1" ) != 0)
     {
         return BAD_REQUEST;
     }
     if( strncasecmp( m_url, "http://", 7 ) == 0 )
     {
         m_url += 7;
         m_url = strchr( m_url, '/' );
     }
     if( ! m_url || m_url[ 0 ] != '/' ){
         return BAD_REQUEST;
     }
     m_check_state = CHECK_STATE_HEADER;
     return NO_REQUEST;
 }

 /* 解析 HTTP 头 */
 http_conn::HTTP_CODE http_conn::parse_headers( char* text )
 {
     /* 遇到一个空行,说明头部字段解析完毕  */
     if( text[ 0 ] == '\0' )
     {
	    if( m_method == HEAD )
	    {
	       return GET_REQUEST;
            }
         /* 如果有请求消息体,则需要读取 m_content_length 字节的消息体,状态机转移到
            CHECK_STATE_CONTENT 状态 */
            if( m_content_length != 0 )
            {
                m_check_state = CHECK_STATE_CONTENT;
                return NO_REQUEST;
            }

            /* 否则说明得到了一个完整的 HTTP 请求 */
            return GET_REQUEST;
     }
     /* 处理头部字段 */
     else if ( strncasecmp( text, "Connection:", 11 ) == 0 )
     {
         text += 11;
         text += strspn( text, " \t" );
         if( strcasecmp( text, "keep-alive" ) == 0 )
         {
             m_linger = true;
         }
     }
     else if ( strncasecmp( text, "Content-Length:", 15 ) == 0 )
     {
         text += 15;
         text += strspn( text, " \t" );
         m_content_length = atol( text );
     }
     else if ( strncasecmp( text, "Host:", 5 ) == 0 )
     {
         text += 5;
         text += strspn( text, " \t" );
         m_host = text;
     }
     else
     {
         cout << " unknow header " << endl;
     }
     return NO_REQUEST;
 }

/* 没有处理,只是判断是否被完整读入了 */
 http_conn::HTTP_CODE http_conn::parse_content( char* text )
 {
     if( m_read_idx >= ( m_content_length + m_checked_idx ) )
     {
         text[ m_content_length ] = '\0';
         return GET_REQUEST;
     }
     return NO_REQUEST;
 }

 /* 主状态机 */
 http_conn::HTTP_CODE http_conn::process_read()
 {
     cout << " process head " << endl;
     LINE_STATUS line_status = LINE_OK;
     HTTP_CODE ret = NO_REQUEST;
     char *text = 0;
     /* while( ( ( m_check_state == CHECK_STATE_CONTENT ) && ( line_status == LINE_OK ) )
     || ( ( line_status == parse_line() ) == LINE_OK ) ) */
    while ( ( ( m_check_state == CHECK_STATE_CONTENT ) && ( line_status == LINE_OK  ) )
                || ( ( line_status = parse_line() ) == LINE_OK ) )
    {
      // start_line 为行在buffer中的起始位置
     text = get_line();
     m_start_line = m_checked_idx;
     cout << " got 1 http line " << text << endl;
     switch ( m_check_state )
     {
         case CHECK_STATE_REQUESTLINE: // 分析请求行
         {
             ret = parse_request_line( text );
             if( ret == BAD_REQUEST )
             {
                 return BAD_REQUEST;
             }
             break;
         }
         case CHECK_STATE_HEADER:
         {
             ret = parse_headers( text );
             if( ret == BAD_REQUEST )
             {
                 return BAD_REQUEST;
             }
             else if ( ret == GET_REQUEST )
             { 
                 return do_request();
             }
             break;
         }
         case CHECK_STATE_CONTENT:
         {
             ret = parse_content( text );
             if( ret == GET_REQUEST )
             {
                 return do_request();
             }
             line_status = LINE_OPEN;
             break;
         }         
         default:
         {
             return INTERNAL_ERROR;
         }
     }        
    } 
    cout << " no request " << endl;
    return NO_REQUEST;
 }

/* 当得到一个完整且正确的 HTTP 请求,就分析目标文件的属性.若目标文件存在且所有用户都可以获取,不是目录,
   则使用 mmap 将其映射到内存地址 m_file_address 处,并告诉调用者获取文件成功 */
 http_conn::HTTP_CODE http_conn::do_request()
 {
     strcpy( m_real_file, doc_root );
     int len = strlen( doc_root );
     strncpy( m_real_file + len, m_url, FILE_NAME_LEN - len - 1 );
     /* 文件不存在 */
     if( stat( m_real_file, &m_file_stat ) < 0 )
     {
	return NO_RESOURCE;
         /* return NO_REQUEST; */
     }
     /* S_IROTH :其他组读权限 */
     if( ! ( m_file_stat.st_mode & S_IROTH ) )
     {
         return FORBIDDEN_REQUEST;
     }
     if( S_ISDIR( m_file_stat.st_mode ) )
     {
         return BAD_REQUEST;
     }
     int fd = open( m_real_file, O_RDONLY );
     /* mmap(2) is a POSIX-compliant Unix system call that maps files or devices into memory.  */
     m_file_address = (char *)mmap( 0, m_file_stat.st_size, PROT_READ, 
     MAP_PRIVATE, fd, 0 );
     close( fd );
     return FILE_REQUEST;
 }

 /* 对内存映射区执行 mummap 操作,即取消映射 */
 void http_conn::unmap()
 {
     if( m_file_address )
     {
         munmap( m_file_address, m_file_stat.st_size );
         m_file_address = 0;
     }
 }
/* 写 HTTP 响应 */
 bool http_conn::write()
 {
     /* printf("write something.."); */
     int temp = 0;
     int bytes_have_send = 0;
     int bytes_to_send = m_write_idx;
     if(bytes_to_send == 0 )
     {
         /* 没有东西可写 */
         modfd( m_epollfd, m_sockfd, EPOLLIN );
         init();
         return true;
     }

     while( 1 )
     {
         temp = writev( m_sockfd, m_iv, m_iv_count );
         if( temp <= -1 )
         {
             /* 若写缓冲没空间,则等待下一轮 EPOLLOUT 时间.虽然再次期间服务器无法
                立即接收同一客户的下一个请求,但这可以保证连接的完整性 */
            if( errno == EAGAIN )
            {
                modfd( m_epollfd, m_sockfd, EPOLLOUT );
                return true;
            }
            unmap();
            return false;
         }
         bytes_to_send -= temp;
         bytes_have_send += temp;
         if( bytes_to_send <= bytes_have_send )
         {
             /* 发送 HTTP 请求成功,根据 Connection 决定是否立即关闭连接 */
             unmap();
             if( m_linger )
             {
                 init();
                 modfd( m_epollfd, m_sockfd, EPOLLIN );
                 return true;
             }
             else
             {
                 modfd( m_epollfd, m_sockfd, EPOLLIN );
                 return false;
             }            
         }
     }
 }

/* 往写缓冲中写入待发送的数据 ( 这函数操作没看懂啊 ) */
bool http_conn::add_response( const char* format, ... )
{
    /* 要发送的数据大于能写入的空间 */
    if( m_write_idx >= WRITE_BUFFER_SIZE )
    {
        return false;
    }
    va_list arg_list;
    va_start( arg_list, format );
    int len = vsnprintf( m_write_buf + m_write_idx, WRITE_BUFFER_SIZE - 1 - m_write_idx, format, arg_list );
    if( len >= ( WRITE_BUFFER_SIZE - 1 - m_write_idx ) )
    {
        return false;
    }
    m_write_idx += len;
    va_end( arg_list );
    return true;
}

bool http_conn::add_status_line( int status, const char* title )
{
    return add_response( " %s %d %s\r\n", "HTTP/1.1", status, title );
}

bool http_conn::add_headers( int content_len )
{
    add_content_length( content_len );
    add_linger();
    add_blank_line();
}

bool http_conn::add_content_length( int content_len )
{
    return add_response( "Content-Length: %d\r\n", content_len );
}

bool http_conn::add_linger()
{
    return add_response( "Connection: %s\r\n", ( m_linger == true ) ? "keep-alive" : "close" );
}

bool http_conn::add_blank_line()
{
    return add_response( "%s", "\r\n" );
}

bool http_conn::add_content( const char* content )
{
    return add_response( "%s", content );
}

/* 根据服务器处理 HTTP 请求的结果,决定返回给客户端的内容 */
bool http_conn::process_write( HTTP_CODE ret )
{
    switch ( ret )
    {
        case INTERNAL_ERROR:
        {
            add_status_line( 500, error_500_title );
            add_headers( strlen( error_500_form ) );
            if( ! add_content( error_500_form ) )
            {
                return false;
            }
            break;
        }
        case BAD_REQUEST:
        {
            add_status_line( 400, error_400_title );
            add_headers( strlen( error_400_form ) );
            if( ! add_content( error_400_form ) )
            {
                return false;
            }
            break;
        }
        case NO_RESOURCE:
        {
            add_status_line( 404, error_404_title );
            add_headers( strlen( error_404_form ) );
            if( ! add_content( error_404_form ) )
            {
                return false;
            }
            break;
        }        
        case FORBIDDEN_REQUEST:
        {
            add_status_line( 403, error_403_title );
            add_headers( strlen( error_403_form ) );
            if( ! add_content( error_403_form ) )
            {
                return false;
            }
            break;
        }
        case FILE_REQUEST:
        {
            add_status_line( 200, ok_200_title );
            if( m_file_stat.st_size != 0 )
            {
                add_headers( m_file_stat.st_size );
                m_iv[ 0 ].iov_base = m_write_buf;
                m_iv[ 0 ].iov_len = m_write_idx;
                m_iv[ 1 ].iov_base = m_file_address;
                m_iv[ 1 ].iov_len = m_file_stat.st_size;
                m_iv_count = 2;
                return true;
            }
            else
            {
                const char* ok_string = "<html><body></body></html>";
                add_headers( strlen( ok_string ) );
                if( ! add_content( ok_string ) )
                {
                    return false;
                }
            }
        }
        default:
        {
           /* break; */
        }
    }
    m_iv[ 0 ].iov_base = m_write_buf;
    m_iv[ 0 ].iov_len = m_write_idx;
    m_iv_count = 1;
    return true;
}

/* 由线程池中的工作线程调用, 这是处理 HTTP 请求的入口函数 */
void http_conn::process()
{
    HTTP_CODE read_ret = process_read();
    if( read_ret == NO_REQUEST )
    {
        /* 重新注册,因为用了 ONSHOT */
        modfd( m_epollfd, m_sockfd, EPOLLIN );
        return;
    }

    bool write_ret = process_write( read_ret );
    if( ! write_ret )
    {
        close_conn();
    }
    /* 同上 */
    modfd( m_epollfd, m_sockfd, EPOLLOUT );
}
