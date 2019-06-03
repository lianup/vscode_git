#ifndef TIME_WHEEL_TIMER
#define TIMER_WHEEL_TIMER
#include <time.h>
#include <netinet/in.h>
#include <iostream>
#endif
/*一个高性能定时器:时间轮(提高了添加定时器的效率)*/
using namespace std;
int const BUFFER_SIZE = 64;
class tw_timer;

/*绑定socket和定时器*/
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    tw_timer *timer;
};

/*定时器类*/
class tw_timer
{
public:
    tw_timer(int rot, int ts) : next(NULL), prev(NULL),rotation(rot), time_slot(ts) {}
    
public:
    int rotation; /*记录定时器在时间轮转多少圈后生效*/
    int time_slot; /*记录定时器属于时间轮上的哪个槽(对应哪个链表)*/
    void (*cb_func) (client_data*); /*回调函数*/
    client_data *user_data; 
    tw_timer *next; /*下一个定时器*/
    tw_timer *prev; /*上一个*/
};

/*时间轮*/
class time_wheel_1
{
private:
    /*时间轮上槽的个数*/
    static const int N = 60;
    /*时间槽间隔(时间轮转动一次时间)为1 s */
    static const int SI = 1;
    /*时间轮的槽,每个元素指向一个链表,链表无序*/
    tw_timer* slots[N];
    /*时间轮的当前槽*/
    int cur_slots;
public:
    time_wheel_1() : cur_slots(0)
    {
        /*初始化每个槽*/
        for(int i = 0; i < N; ++i)
        {
            slots[i] = NULL;
        }
    }

    ~time_wheel_1()
    {
        for(int i = 0; i < N; ++i)
        {
            /*遍历每个槽,释放每个槽的定时器*/
            tw_timer *temp = slots[i];
            while(temp)
            {
                slots[i]->next = temp->next;
                delete temp;
                temp = slots[i];
            }
        }
    }

    /*根据定时器timeout创建一个定时器,并插入槽中*/
    tw_timer* add_timer(int timeout)
    {
        if(timeout < 0)
        {
            return NULL;
        }
        int ticks = 0;
        if(timeout < SI)
        {
            ticks = 1;
        }
        else
        {
            ticks = timeout / SI;
        }
        /*计算待插入的定时器在时间轮转动多少圈后被触发*/
        int rotation = ticks / N;
        /*计算应插入槽
        注意:这里我没有把ticks%,因为我觉得上面已经计算过了*/
        int ts = (cur_slots + ticks ) % N;
        /*创建新的定时器*/
        tw_timer *timer = new tw_timer(rotation, ts);

        /*把定时器插入槽中*/
        if(!slots[ts])
        {
            cout << "add timer,rotation is " << rotation << " ts is " << ts
             << "cur_slots is " << cur_slots << endl;
             slots[ts] = timer;
        }
        else
        {
            timer->next = slots[ts];
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }
        return timer;
    }

    /*删除目标定时器,时间复杂度为O(1),此函数的目标定时器就是在链表中的.返回下一个timer*/
    tw_timer* delete_timer(tw_timer *timer)
    {
        if(!timer)
        {
            return;
        }
        /*timer所在槽*/
        int ts = timer->time_slot;
        tw_timer *next = NULL;
        /*timer为头结点*/
        if(timer == slots[ts])
        {
            slots[ts] = timer->next;
            /*注意要先判空*/
            if(slots[ts])
            {
                next = slots[ts];
                slots[ts]->prev = NULL;
            }
            delete timer;
        }
        else
        {
            timer->prev->next = timer->next;
            if(timer->next)
            {
                next = timer->next;
                timer->next->prev = timer->prev;
            }
            delete timer;
        }
        return next;
    }

    /*SI 时间到后,调用该函数,时间轮向前滚动一个槽的间隔
    即要先处理当前槽的定时器任务事件,然后把cur_slot指向下一个槽*/
    void tick()
    {
        tw_timer *temp = slots[cur_slots];
        cout << "cur_slots:" << cur_slots << endl;
        while(temp)
        {
            cout << "tick the time once" << endl;
            if(temp->rotation > 0)
            {
                --(temp->rotation);
                temp = temp->next;
            }
            /*执行定时器任务,并删除此定时器*/
            else
            {
                temp->cb_func(temp->user_data);
                temp = delete_timer(temp);
            }
        }
        cur_slots = ++cur_slots % N;
    }

};


