#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <forward_list>
#include <string>
#include <array>
using namespace std;

/**
 * 测试元素操作
 */
int main(){
	vector<int> v;
	deque<int> d;
	v.push_back(10);
	// 造成数据移动,list,deque,forward_list支持
	d.push_front(2);
	// 将元素插入到迭代器之前，list,deque,forward_list,vector,string
	// 返回指向第一个新元素的迭代器
	v.insert(v.begin(),5);
	// 批量插入
	v.insert(v.end(),3,11);
	// 传入参数，构造元素并插入
	v.emplace_back(4);
	// 返回首元素and尾元素的引用,使用前确保容器不为空
	auto begin = v.front(),end = v.back();
	// 返回容器某位置元素的引用，at会报错
	cout<<v[0]<<" "<<v.at(0)<<endl;
	//删除元素的操作,删除前需确保元素存在
	//string,vector不支持
	d.pop_front();
	v.pop_back();
	// 删除迭代器所指元素，返回指向最后一个被删的后面的元素的迭代器；forward_list有特殊的erase操作
	v.erase(v.begin());
	//范围
	v.erase(v.begin(),v.end());
	//清除所有元素
	v.clear();
	// 调整大小，改变容器元素数量而非容量；
	// 若变小可能使得指向被删元素的引用、迭代器、指针失效；
	v.resize(10,5);
	return 0;
}

/**
 * 测试顺序容器类型
 */
int main1(){
	// 单向链表
	forward_list<int> flist(10);
	forward_list<int> flist2(12);
	// 除array外，只改变指向内部数据结构的指针，为O（1）
	swap(flist,flist2);
	// 容器比较，必须容器类型and数据类型相同，且数据类型定义了比较运算符
	cout<<(flist == flist2)<<endl;
	// 定长数组，只能读不能改             
	array<int,10> arr;
	list<string> names;
	vector<const char*> test;
	// assign允许从不同但相容的类型赋值
       names.assign(test.cbegin(),test.cend());	
	return 0;
}
