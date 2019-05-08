#include <vector>
#include <iostream>
#include <string>
#include <iomanip>
using namespace std;


/**
 * 未格式化的输入、输入操作
 */
int main(){
    char ch;
    cin.get(ch);
    cout.put(ch);
    return 0;
}

/**
 * 操作符：一个操作符是一个函数or一个对象，会影响流的状态，并且能够用作
 * 输入或输出运算符的运算对象。例如endl
 */
int main1(){
    // bool操作符
    cout << boolalpha << true << noboolalpha << true << endl;

    // 指定整形值的进制
    cout << oct << 20 << hex << 1024 << dec << 20 << endl;

    // 恢复流状态
    cout << noshowbase;

    // 控制浮点数格式
    // 控制输出精度为12位数字
    cout << cout.precision(12) << 12232.333 << endl;
    cout << setprecision(3) << 4534.343 << endl;

    return 0;
}   