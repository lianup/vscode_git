#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>


/* 用一个线程统一处理一个进程中线程的所有信号 */

#define handle_error_en( en, msg ) \
    do { errno = en;perror( msg );exit( EXIT_FAILURE ); } while ( 0 )

/* 统一处理信号的线程处理函数 */
static void *sig_thread( void *arg )
{
    sigset_t *set = (sigset_t *)arg;
    int s, sig;
    for( ;; )
    {
        /* 步骤二,调用 sigwait 等待信号 */
        if( s != 0 )
        {
            handle_error_en( s, "sigwait" );
            printf( "signal handling thread got signal \n" );
        }
    }
}


int main( int argc, char *argv[] )
{
    pthread_t thread;
    sigset_t set;
    int s;
    /* 步骤一,在主线程中设置信号掩码,所有新线程创建的子线程都将自动继承这个信号掩码.所有线程
    都不会响应被屏蔽的信号了. */
    sigemptyset( &set );
    sigaddset( &set, SIGQUIT );
    sigaddset( &set, SIGUSR1 );
    s = pthread_sigmask( SIG_BLOCK, &set, NULL );
    if( s != 0 )
    {
        handle_error_en( s, "pthread_sigmask" );
    }
    s = pthread_create( &thread, NULL, &sig_thread, (void *) &set );
}