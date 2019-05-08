#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <sstream>

using namespace std;


/**
 * 终止递归并打印最后一个元素的函数
 */
template <typename T>
ostream &print(ostream &os, const T &t){
	return os << t;
}

template <typename T, typename ... Args>
ostream &print(ostream &os, const T &t, const Arg&... rest){ // 扩展Args
	os << t << " ";
	// 递归调用，打印其他实参
	return print(os, rest...); // 扩展 rest
}


/**
 * 可变参数模板
 * args：一个模板参数包；rest是一个函数参数包
 */
template <typename T, typename ... Args>
void foo(const T &t, const Args& ... rest){

	// 用sizeof打印类型参数和函数参数的数目
	cout << sizeof...(Args) << endl;
	cout << sizeof...(Args) << endl;
}

/**
 * 此函数模板能用于任何类型，包括指针，包括引用
 *此函数生成一个对象对应的string表示
 */
template <typename T> string debug_rep(const T &t){
	ostringstream ret;
	ret << t;
	// 返回ret绑定的string的一个副本
	return ret.str();
}

/**
 * 打印指针的版本
 */
template <typename T> string debug_rep(T *p){
	ostringstream ret;
	ret << "pointer:" << p;
	if(p){
		ret << " " << debug_rep(*p);
	}else{
	ret << "null pointer";
	}
	return ret.str();
}

/**
 * 普通非模板版本
 */
string debug_rep(const string &s){
	return  "common " + s;
}

int main(){

	string s("hi");
	// 若一个非函数模板and一个函数模板能对一个调用提供同样好的匹配，则编译器会选择非模板版本。
	cout << debug_rep(s) << endl;
	// 编译器会选择第二个版本，即更特例化的版本。
	cout << debug_rep(&s) << endl;
	return 0;

}
