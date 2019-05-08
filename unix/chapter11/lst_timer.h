#include "header.h"
#include <time.h>
#ifndef LST_TIMER
#define LST_TIMER
using namespace std;

/**
 * 实现一个简单的升序定时器链表．其将其中的定时器按照超时时间升序．
 * 其实就是一个链表的操作,再加上执行的操作tick
 * ps:编译头文件的时候，没有ｍａｉｎ函数，记得加　-c　参数
 */
const int BUFFER_SIZE = 1024;
class util_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer *timer;
};

/**
 * 定时器类
 */
class util_timer
{
public:    
    util_timer() : prev(NULL), next(NULL) { }
    time_t expire; /*任务的超时时间，这里使用绝对时间*/
    void (*cb_func)(client_data*); /*任务回调函数*/
    /*回调函数处理的客户数据，由定时器的执行者传递给回调函数*/
    client_data* user_data;
    util_timer* prev; /*指向上一个定时器*/
    util_timer* next;/*指向下一个定时器*/
};

/**
 * 定时器链表，它是一个升序，双向链表，带有头和尾节点
 */
class sort_timer_lst
{
public:
    sort_timer_lst() : head(NULL), tail(NULL) {}
    ~sort_timer_lst()
    {
        util_timer* tmp = head;
        while(tmp)
        {
            head = tmp->next;
            delete tmp;
            tmp = head;
        }
    }
    void add_timer(util_timer* timer)
    {
        if(!timer)
        {
            return;
        }
        if(!head)
        {
            head = tail = timer;
            return;
        }
        if(timer->expire < head->expire)
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }
        add_timer(timer, head);
    }

    /*只考虑某个定时器需要往链表尾部移动的情况*/
    void adjust_timer(util_timer* timer)
    {
        if(!time)
        {
            return;
        }
        util_timer* tmp = timer->next;
        /*若目标处在链表尾部，或者expire < next->expire，不用调整*/
        if(!tmp || (timer->expire < tmp->expire))
        {
            return;
        }
        /*若目标为头结点*/
        if(timer == head)
        {
            head = head->next;
            head->prev = NULL;
            timer->next = NULL;
            add_timer(timer, head);
        }
        else
        {
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            add_timer(timer, timer->next);
        }
    }
    void del_timer(util_timer* timer)
    {
        if(!timer)
        {
            return;
        }
        /*只有一个定时器时*/
        if((timer == head) && (timer == tail))
        {
            delete timer;
            head = NULL;
            tail = NULL;
            return;
        }

        /*目标为头结点*/
        if(timer == head)
        {
            head = head->next;
            head->prev = NULL;
            delete timer;
            return;
        }
        /*目标为尾节点*/
        if(timer == tail)
        {
            tail = tail->prev;
            tail->next = NULL;
            delete timer;
            return;
        }
        /*若目标为其中一个*/
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    }

    /*信号出发时，处理定时器事件的核心函数*/
    void tick()
    {
        if(!head)
        {
            return;
        }
        cout << "timer tick" << endl;
        time_t cur = time(NULL); /*获得系统当前时间*/
        util_timer* tmp = head;
        while(tmp)
        {
            if(cur < tmp->expire)
            {
                break;
            }
            /*若超时，则调用回调函数，执行定时任务*/
            tmp->cb_func(tmp->user_data);
            /*执行完之后从链表删除，并重置链表头*/
            head = tmp->next;
            if(head)
            {
                head->prev = NULL;
            }
            delete tmp;
            tmp = head;
        }
    }
private:
    /*一个重载的辅助函数，被共有的add_timer和adjust_timer调用
    该函数表示将目标添加到节点lst_head之后的部分链表中*/
    void add_timer(util_timer* timer, util_timer* lst_head)
    {
        util_timer* prev = lst_head;
        util_timer* tmp = prev->next;
        while(tmp)
        {
            if(timer->expire < tmp->expire)
            {
                prev->next = timer;
                timer->prev = prev;
                tmp->prev = timer;
                timer->next = tmp;
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }
        /*若目标应该为链表尾部*/
        if(!tmp)
        {
            prev->next = timer;
            timer->prev = prev;
            timer->next = NULL;
            tail = timer;
        }
    }
        util_timer* head;
        util_timer* tail;    
    
}; 
#endif // !LST_TIMER
