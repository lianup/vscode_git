#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <utility>
using namespace std;

// 枚举的前置声明
enum intValues : unsigned long long;
/**
 * 枚举类型
 * 成员是const,提供的初始值必须为常量
 */
int main(){

    // 定义限定作用域的枚举类型
    enum class open_modes {input, output, append };
    // 定义不限定作用域的枚举类型
    enum color {red, yellow, green};
    //emnum stoplight{red, yellow, green}; 错误,重复定义了枚举成员
    color eyes = green;
    int i = color::green;
 //   int j = open_modes::input; 错误,只有不限定作用域的枚举类型的对象会自动转换整形
    // 一般来说,枚举值从0开始递增,也可以自定义
    // c++11:可以:指定默认类型(枚举值实质为某种整数类型)
    enum class initTypes : unsigned long long{
        charTyp = 8, shortTyp = 16, intTyp = 16
     };
    return 0;
}