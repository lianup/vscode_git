#ifndef STRVEC_H
#define STRVEC_H
#include <string>
#include <memory>
#include <iostream>
#include <new>
/**
 * 创建一个自己用于存放string的动态内存管理类（类似vector）
 * 成员:elements(指向首元素的指针) frist_free（指向最后一个元素的后一个位置） cap（指向分配的的内存末尾之后的位置）
 */
class StrVec{
public:
	StrVec():elements(nullptr), first_free(nullptr), cap(nullptr){}
	StrVec(const StrVec&);
//	StrVec(const StrVec&&); 移动构造函数
	StrVec &operator=(const StrVec&);
	~StrVec();
	void push_back(const std::string&);
	size_t size() const {return first_free - elements;}
	size_t capacity() const {return cap - elements;}
	std::string *begin() const {return elements;}
	std::string *end() const {return first_free;}
	std::pair<std::string*,std::string*> alloc_n_copy(const std::string *b,const std::string *e);
private:
	static std::allocator<std::string> alloc;
	void chk_n_alloc(){
		if(size() == capacity()){
			reallocate();
		}
	}
	void free();// 销毁元素并释放内存
	void reallocate();// 获取更多内存并拷贝已有元素
	std::string *elements;
	std::string *first_free;
	std::string *cap;
};
#endif 
