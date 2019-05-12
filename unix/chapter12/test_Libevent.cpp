#include <sys/signal.h>
#include <event.h>
#include <iostream>
using std::cout;
using std::endl;


/*使用Libevent的简单例子
 * 注意编译时要 -levent*/

void signal_cb( int fd, short event, void* argc )
{
    struct event_base *base = ( event_base* )argc;
    struct timeval delay = { 2, 0 };
    cout << "caught a interupt signal " << endl;
    event_base_loopexit( base, &delay);
}

void timeout_cb( int fd, short event, void *argc)
{
    cout << "time out" << endl;
}

int main()
{
    /* event_base 相当于 Reactor 实例 */
    struct event_base *base = event_init();

    /* evsignal_new 创建信号事件处理器, base 指定从属的 Reactor
 *     x 指定关联的句柄, cb 为回调函数 */
    struct event *signal_event = evsignal_new( base, SIGINT, signal_cb, base);
    event_add( signal_event, NULL );

    /* evtimer_new 创建定时事件处理器 */
    timeval tv = { 1, 0 };
    struct event *timeout_event = evtimer_new( base, timeout_cb, NULL );
    event_add( timeout_event, &tv);

    /*执行事件循环*/
    event_base_dispatch( base );

    /*释放系统资源*/
    event_free( timeout_event );
    event_free( signal_event );
    event_base_free( base );
}
