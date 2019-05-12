#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../chapter14/locker.h"

/* 半同步/半反应堆线程池的实现:更高效,添加一个工作队列,工作县城从队列中取得任务.不用
主线程轮训去分配任务. */

/* 线程池类 */
template < typename T >
class threadpool
{
private:
    int m_thread_number; /* 线程池中的线程数 */
    int m_max_requst; /* 请求队列中的允许的最大请求数 */
    pthread_t *m_threads; /* 描述线程池的数组 */
    std::list< T* > m_workqueue; /* 请求队列 */
    locker m_queuelocker; /* 保护请求队列的互斥锁 */
    sem m_queuestat; /* 是否有任务需要处理 */
    bool m_stop; /* 是否结束线程 */

private:
    /* 工作线程运行的函数,它不断从工作队列中取出任务并执行 */
    static void* worker( void *arg );
    void run();

public:
    threadpool( int thread_number = 0, int max_request = 10000 );
    ~threadpool();
    /* 往请求队列中添加任务 */
    bool append( T *requst );

};

template< typename T >
threadpool< T >::threadpool( int thread_number, int max_request ) : 
    m_thread_number( thread_number ), m_max_requst( max_request ),
    m_stop( false ), m_threads( NULL ) 
    {
        if( ( thread_number <= 0 ) || max_requst <= 0 )
        {
            throw std::exception;
        }
        m_threads = new pthread_t[ thread_number ];
        if( !m_threads )
        {
            throw std::exception();
        }

        /* 创建 thread_number 个线程,并将它们都设置为脱离函数 */
        for( int i = 0; i < thread_number; ++i )
        {
            cout << "create the " << i << " thread" <<  endl;
            if( pthread_create( m_threads + i, NULL, worker, this ) != 0 )
            {
                delete [] m_threads;
                throw std::exception();
            }
            /* 脱离线程:不向主线程返回信息,不需要主线程等待
            完成工作后自行消失? */
            if( pthread_detach( m_threads[ i ] ) )
            {
                delete [] m_threads;
                throw std::exception();
            }
        }
    }

template< typename T >
threadpool< T >::~threadpool()
{
    delete [] m_threads;
    m_stop = true;
}

/* 往请求队列中添加任务 */
template< typename T >
bool threadpool< T >::append( T *request )
{
    /* 操作工作队列时必须加锁 */
    m_queuelocker.lock();
    if( ( m_workqueue.size >= m_max_requst ) )
    {
        m_queuelocker.unlock;
        return false;
    }
    m_workqueue.push_back( request );
    m_queuelocker.unlock;
    /* 增加信号量, V 操作 ? */
    m_queuestat.post();
    return true;
}

/* 工作线程运行的函数,它不断从工作队列中取出任务并执行 */
template< typename T >
void* threadpool< T >::worker( void *arg )
{
    threadpool *pool = ( threadpool* )arg;
    pool->run();
    return pool;
}

/* 工作线程的 run 函数: 把工作队列中的任务取出来运行 */
template< typename T >
void* threadpool< T >::run()
{
    while( !m_stop )
    {
        m_queuestat.wait();
        m_queuelocker.lock();
        if( m_workqueue.empty() )
        {
            m_queuelocker.unlock();
            continue;
        }
        T *request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if( !request )
        {
            continue;
        }
        requst->process();
    }
}
#endif // !THREADPOOL_H#define THREADPOOL_H