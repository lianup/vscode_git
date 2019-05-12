#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <wait.h>
#include <stdlib.h>

using std::cout;
using std::endl;

/* 在多线程程序中调用fork,得到的子进程是调用 fork 的那个线程的完整复制.
子进程将自动继承父进程中互斥锁中的状态,这样就可能导致死锁 */

pthread_mutex_t mutex;

/* 子进程运行函数.它先获得互斥锁,然后再释放. */
void* child_thread_run( void *arg )
{
    cout << "child lock the mutex" << endl;
    pthread_mutex_lock( &mutex );
    sleep(5);
    pthread_mutex_unlock( &mutex );
}


int main()
{
    pthread_mutex_init( &mutex, NULL );
    pthread_t id;
    /* 创建子线程 */
    pthread_create( &id, NULL, child_thread_run, NULL );

    /* 主线程暂停1 s ,确保在执行 fork 操作前,子线程已经执行并且获得 mutex */
    sleep( 1 );
    int pid = fork();
    if( pid < 0 )
    {
        pthread_join( id, NULL );
        pthread_mutex_destroy( &mutex );
        return 1;
    }
    else if( pid == 0 )
    {
        cout << "child process want to get lock" << endl;
        pthread_mutex_lock( &mutex );
        cout << "can not run to here" << endl;
        pthread_mutex_unlock( &mutex );
        exit( 0 );
    }
    else
    {
        wait( NULL );
    }
    pthread_join( id, NULL );
    pthread_mutex_destroy( &mutex );
    return 0;
}