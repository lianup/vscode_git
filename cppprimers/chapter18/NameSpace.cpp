#include <iostream>
#include <vector>
#include <string>
#include "Test.h"
// using 指示(应避免使用),区别using声明:声明一次只引入命名空间的一个成员
using namespace std;
using namespace cpp_primer;
/**
 * 命名空间的定义：关键字namespace + 名字
 * 其定义可以不连续，下述可以是新建一个namespace or 向已有namespace中添加元素
 */

/**
 * 未命名的命名空间:拥有静态生命周期,程序结束时销毁
 * Cpp中用其来代替在文件中进行静态声明(声明为static的全局实体在其所在的文件外不可见)
 */
namespace{

    int i;
}
// 为命名空间设置别名
namespace primer = cpp_primer;

int main(){
    cpp_primer::Query q = cpp_primer::Query();
    // 内联命名空间可直接用外层嵌套名字
    cpp_primer::My my = cpp_primer::My();
    return 0;
}