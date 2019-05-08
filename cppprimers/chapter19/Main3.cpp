#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <utility>
using namespace std;
/**
 * union:一种特殊的类,任意时刻只有一个数据成员可以有值(其他为未定义)
 * 分配给union的空间为它的最大数据成员
 */
union Token
{
    char cval;
    int ival;
    
};


int main(){
    Token first_token = {'A'};
    Token last_token;
    Token *pt = new Token();
    last_token.cval = 'z';
    pt->ival = 42;


    /**
    * 匿名union
    */
    union {
        char cv;
        int iv;
    };
    cv = 'c';
    iv = 22;
    cout << cv << " " << iv << endl;


    return 0;
}