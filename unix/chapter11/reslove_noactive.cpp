#include "header.h"
#include "lst_timer.h"

/**
 * 使用alarm函数周期性地出发SIGALRM信号,关闭非活动的链接
 */

int const FD_LIMIT = 65535;
int const MAX_EVENT_NUMBER = 1024;
int const TIMESLOT = 5;
static int pipefd[2];



