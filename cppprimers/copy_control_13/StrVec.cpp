#include "StrVec.hpp"
#include <utility>
using namespace std;
// 编译的时候：g++ -o StrVec.cpp StrVec.hpp （从右边开始）
class StrVec;
void StrVec::push_back(const string& s){
	chk_n_alloc();
	// 在first_free 指向的元素中构造s的副本
	alloc.construct(first_free++,s);
}

pair<string*,string*> StrVec::alloc_n_copy(const string *b, const string *e){
	auto data = alloc.allocate(e-b);
//	return {data,uninitialized_copy(b, e, data)};
	return {data,copy(b,e,data)};
}
/**
 * 销毁元素，并释放分配的内存空间。
 */
void StrVec::free(){
	//不能传给destroy一个空指针
	if(elements){
		for(auto p = first_free; p != elements;){
			alloc.destroy(--p);
		}
		// 释放分配的内存空间
		alloc.deallocate(elements, cap - elements);	
	}
}

// 拷贝控制函数
StrVec::StrVec(const StrVec &s){
	auto newdata = alloc_n_copy(s.begin(),s.end());
	elements = newdata.first;
	first_free = newdata.second;
}

// 移动构造函数,其不分配任何资源。故通常不抛出异常。
//StrVec::StrVec(const StrVec &&s) noexcept : elements(s.elements), first_free(s.first_free), cap(s.cap){
//	// 移后源对象必须可析构
//	s.elements = s.first_free = s.cap = nullptr;
//}

// 析构函数
StrVec::~StrVec(){free();}

// 拷贝赋值运算符
StrVec &StrVec::operator=(const StrVec &rhs){
	auto data = alloc_n_copy(rhs.begin(),rhs.end());
	free();
	elements = data.first;
	first_free = data.second;
	cap = data.second;
	return *this;
}

/**
 * 重分配内存
 */
void StrVec::reallocate(){
	auto newcap = size() ? 2 * size() : 1;
	// 分配新内存
	auto newdata = alloc.allocate(newcap);
	// 指向新数组中的下一个空闲位置
	auto dest = newdata;
	// 指向旧数组中的下一个元素
	auto elem = elements;
	for(size_t i = 0;i != size();++i){
		//调用move(新标准)，使用string的移动构造函数（拷贝字符数组的指针而不是拷贝字符）
		alloc.construct(dest++,std::move(*elem++));
	}
	//释放旧内存
	free();
	elements = newdata;
	first_free = dest;
	cap = elements + newcap;
}

int main(){
	return 0;

}


