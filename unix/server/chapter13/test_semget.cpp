#include <sys/sem.h>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

using std::cout;
using std::endl;

/**
 * semget () 给 key 参数传递 IPC_PRIATE (值为 0 ),无论信号量是否存在,都会
 * 创建一个新的信号量.
 * 在父,子进程中使用一个 IPC_PRIVATE 信号量来同步
 */

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short int* array;
    struct seminfo *__buf;
};

/* op 为 -1 时执行 P 操作, op为 1 时执行 V 操作*/
void pv(int sem_id, int op)
{

}

int main(int argc, char *argv[]){
    int sem_id = semget( IPC_PRIVATE, 1, 0666);

    union semun sem_un;
    sem_un.val = 1;
    /*设置值*/
    semctl( sem_id, 0, SETVAL, sem_un);
    
    pid_t id = fork();
    if( id < 0 )
    {
        return 1;
    }
    /*返回0为子线程正在执行*/
    else if( id == 0 )
    {
        cout << "child try to get binary sem" << endl;
        /*父子线程间共享 IPC_PRIVATE 信号量的关键在于二者都可以操作信号量的标识符*/
        pv( sem_id, -1 );
        exit(0);
    }
    else
    {
        cout << "parent now " << endl;
        pv( sem_id, -1 );
        cout << "parent get the sem and would release it after 5 seconds" << endl;
        sleep( 5 );
        pv( sem_id, 1);
    }
    waitpid( id, NULL, 0);
    semctl( sem_id, 0, IPC_RMID, sem_un );
    return 0;
}