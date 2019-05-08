#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <tuple>
using namespace std;


/**
 * tuple 是类似pair的模板，但一个tuple可以拥有任意数量的成员。
 * 可用于：将一些数据组合成单一对象，但又不用定义一个新的数据结构。
 * 实际：从一个函数返回多个值
 */


/**
 * 关系和相等运算符
 */
int main(){

	tuple<string, string> duo("1", "2");
	tuple<size_t, size_t> twoD(1, 2);
	bool b;
//	bool b = (duo == twoD); // 错误，不能比较size_t string
	tuple<size_t, size_t, size_t> threeD(1, 2, 3);
//	b = (twoD < threeD);// 错误，成员数量不同
	tuple<size_t, size_t> origin(0, 0);
	b = (origin < twoD); // 对
	return 0;
}

/**
 * tuple定义和基本使用
 */
int main1(){

	// 定义一个tuple，必须使用直接初始化.
	tuple<size_t, size_t, size_t> threeD{1,2,3};
	auto item = make_tuple("abc", 3, 20.00);

	// 访问tuple的第一个成员
	// 尖括号中的值为第x个
	auto book = get<0>(item);
	
	// 若不确定tuple准确的类型信息，可用两个辅助模板来查询其成员的数量和类型
	typedef decltype(item) trans; // trans 为 item 的类型
	size_t sz = tuple_size<trans>::value;
	tuple_element<1, trans>::type cnt = get<1>(item);
	return 0;
}
