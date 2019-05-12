#include <pthread.h>
#include <unistd.h>
#include <iostream>

using std::cout;
using std::endl;

/* 死锁举例 */

int a = 0;
int b = 0;
pthread_mutex_t mutex_a; /* 互斥锁 a */
pthread_mutex_t mutex_b; /* 互斥锁 b */

/* 子线程运行函数:先获取 b ,再获取 a */
void *child_thread_run( void* arg )
{
    pthread_mutex_lock( &mutex_b );
    cout << "child get b, waiting for a" << endl;
    sleep( 5 );
    ++b;
    pthread_mutex_lock( &mutex_a );
    b += a++;
    pthread_mutex_unlock( &mutex_a );
    pthread_mutex_unlock( &mutex_b );
    /* 确保线程安全退出 */
    pthread_exit( 0 );
}

int main()
{
    /* 子线程 id */
    pthread_t id;

    /* 初始化互斥锁 */
    pthread_mutex_init( &mutex_a, NULL );
    pthread_mutex_init( &mutex_b, NULL );
    /* 创建子线程 */
    pthread_create( &id, NULL, child_thread_run,NULL );

    pthread_mutex_lock( &mutex_a );
    cout << "parent get a and waiting fo b" << endl;
    sleep( 5 );
    ++a;
    pthread_mutex_lock( &mutex_b );
    a += b++;
    pthread_mutex_unlock( &mutex_b );
    pthread_mutex_unlock( &mutex_a );

    /* 等待子线程执行完 */
    pthread_join( id, NULL );
    pthread_mutex_destroy( &mutex_a );
    pthread_mutex_destroy( &mutex_b );
    
    return 0;
}