#include <unistd.h>
#include <iostream>
using namespace std;

/**
 * 以root用户启动的进程切换到普通用户运行
 */
static bool switch_to_normal_user(uid_t user_id, gid_t gp_id){
    // 确保目标用户不是root
    if((user_id == 0) && (gp_id == 0)){
        return false;
    }
    gid_t gid = getgid();
    uid_t uid = getuid();
    // 确保当前用户是合法用户:root 或者 目标用户
    if(((gid != 0) || (uid != 0)) && ((gid != gp_id) || (uid != user_id))){
        return false;
    }

    // 若不是root用户,则已经是目标用户
    if(uid != 0){
        return true;
    }
    // 切换到目标用户
    if((setgid(gp_id) < 0 ) || (setuid(user_id) < 0)){
        return false;
    }
    return true;
}

int main(){

    return 0;
}