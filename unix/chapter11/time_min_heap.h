#ifndef TIME_MIN_HEAP
#define TIME_MIN_HEAP

#include <iostream>
#include <netinet/in.h>
#include <time.h>
using std::exception;
using std::cout;

int const BUFFER_SIZE = 64;
class heap_timer; 

/*用最小堆实现时间轮,插入O(logn),删除O(1)(延迟删除),执行O(1)*/

/*绑定socket和定时器*/
struct client_data{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    heap_timer *timer;
};

/*定时器类*/
class heap_timer{
public:
    heap_timer(int delay)
    {
        expire = time(NULL) + delay;
    }

public:
    time_t expire; /*定时器生效的绝对时间*/
    void (*cb_func)(client_data*); /*回调函数*/
    client_data *user_data;
};

/*时间堆类*/
class time_heap
{
private:
    /*下降操作*/
    void percloate_down(int hole)
    {
        heap_timer* temp = array[hole];
        int child = 0;
        for( ; (hole * 2 + 1) <= (cur_size - 1); hole = child)
        {
            child = hole * 2 + 1;
            if(child < (cur_size - 1) && (array[child + 1]->expire < 
            array[child]->expire ))
            {
                ++child;
            }
            if(array[child]->expire < temp->expire)
            {
                array[hole] = array[child];
            }
            else
            {
                break;
            }
        }
        array[hole] = temp;
    }

    /*数组扩容两倍*/
    void resize() throw (exception)
    {
        heap_timer** temp = new heap_timer* [2*capacity];
        if( ! temp ){
            throw exception();
        }
        for( int i = 0; i < 2 * capacity; ++i)
        {
            temp[i] = NULL;
        }
        capacity *= 2;
        for(int i = 0; i < cur_size; ++i)
        {
            temp[i] = array[i];
        }
        delete [] array;
        array = temp;
    }

private:
    heap_timer** array; /*堆数组*/
    int capacity;
    int cur_size; /*实际个数*/

public:
    time_heap(int cap) throw (exception) : capacity(cap), cur_size(0)
    {
        array = new heap_timer* [capacity];
        if( ! array)
        {
            throw exception();
        }
        for(int i = 0; i < cap; ++i)
        {
            array[i] = NULL;
        }
    }

    /*已有数组初始化堆*/
    time_heap(heap_timer **init_array, int size, int capacity) throw (exception) 
    : cur_size(size), capacity(capacity)
    {
        if(capacity < size)
        {
            throw exception();
        }
        array = new heap_timer* [capacity];
        if( ! array)
        {
            throw exception();
        }
        for( int i = 0; i < capacity; ++i)
        {
            array[i] = NULL;
        }
        if(size != 0)
        {
            /*初始化堆数组*/
            for( int i = 0; i < size; ++i)
            {
                array[ i ] = init_array[ i ];
            }
            /*进行下滤操作,过滤堆*/
            for( int i = (cur_size - 1) / 2; i >= 0; --i)
            {
                percloate_down(i);
            }
        }
    }

    ~time_heap()
    {
        for( int i = 0; i < cur_size; ++i)
        {
            delete array[i];
        }
        delete [] array;
    }

public:
    /*添加目标定时器timer*/
    void add_timer(heap_timer *timer) throw (exception)
    {
        if( ! timer)
        {
            throw exception();
        }
        if(cur_size >= capacity)
        {
            resize();
        }
        /*hole是原始插入位置*/
        int hole = ++cur_size;
        int parent = 0;
        for(; hole > 0; hole = parent)
        {
            parent = (hole - 1) / 2;
            if(array[parent]->expire < array[hole]->expire)
            {
                break;
            }
            array[hole] = array[parent];
        }
        array[hole] = timer;
    }

    /*删除timer*/
    void del_timer(heap_timer *timer) throw (exception)
    {
        if( ! timer)
        {
            return;
        }
        /*延迟销毁.仅将目标定时器的回调函数置空.但是导致数组膨胀*/
        timer->cb_func = NULL;
    }

    bool empty() const
    {
        return cur_size == 0;
    }


    /*获得顶部 timer */
    heap_timer* top() const
    {
        if (empty() )
        {
            return NULL;
        }
        return array[0];
    }

    /*删除堆顶部定时器*/
    void pop_timer()
    {
        if(empty())
        {
            return;
        }
        if(array[0])
        {
            delete array[0];
            array[0] = array[--cur_size];
            /*重新调整*/
            percloate_down( 0 );
        }
    }

    /*心搏函数,即处理到期的堆定时器*/
    void tick()
    {
        heap_timer *temp = array[0];
        time_t cur = time(NULL);
        while( !empty() )
        {
            if( ! temp)
            {
                break;
            }
            /*若堆顶定时器未到期,退出循环*/
            if(temp->expire > cur)
            {
                break;
            }
            /*执行并删除*/
            if(temp->cb_func)
            {
            temp->cb_func(temp->user_data);
            }
            pop_timer();
            temp = array[0];
        }
    }
};

#endif // !TIME_MIN_HEAP