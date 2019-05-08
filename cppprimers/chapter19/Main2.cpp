#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <utility>
using namespace std;

class Screen
{  

public:
    typedef string::size_type pos;
    Screen(/* args */);
    ~Screen();
    char get() const;
    char get(pos ht, pos wd) const;
    Screen::Screen(/* args */)
    {
    }

    Screen::~Screen()
    {
    }
private:
    string contents;
    pos cursor;
    pos height,width;



int main(){

//  定义一个指向常量(非常量)对象的string成员
    const string Screen::*pdata;
    pdata = &Screen::contents;
    //c++11中最方便的做法是是直接使用auto或者decltype
    auto data = &Screen::contents;
    return 0;
}