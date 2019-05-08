#include <iostream>
#include <string>
#include <vector>
#include <memory>
// 包括bad_alloc和nothrow
#include <new>
using namespace std;




/**
 * 动态数组
 * 推荐使用容器，而非动态数组
 */
 int main(){
 
	 // 返回一个数组元素类型的指针，并非数组类型。
	 int *pia = new int[3]{1,2,3};
	 // 合法，但不能解引用；返回一个合法的非空指针。此指针就像尾后迭代器一样。
	 char *cp = new char[0];
	 // 元素逆序销毁
	 delete [] pia;
	 //标准库提供的管理数组的unique版本,不支持成员访问运算反（.和箭头运算符）
	 unique_ptr<int[]> up(new int[10]);
	 // 自动销毁
	 up.release();
	 // 要使用shared管理，需自定义删除器
	 shared_ptr<int> sp(new int[10],[](int *p){delete [] p;});
	 // shared未定义下标运算符
	 for(int i = 0;i != 10;++i){
		 // 使用get获取内置指针
		 *(sp.get()+i) = i;
	 }
	 sp.reset();
	 
	 // allocator使得内存分配和对象构造分离开来。它分配的内存是原始的，未构造的。
	 allocator<string> alloc;
	 // 分配一段原始内存
	 auto const p = alloc.allocate(5);
	 auto q = p;
	 // q指向最后构造的元素之后的位置
	 alloc.construct(q++,"hi");
	 cout << *p << endl;
	 // 错误，指向未构造的内存
//	 cout << *q << endl;
	 while(q != p){
		 // 销毁构造的string
		 alloc.destroy(--q);
	 }
	 // 释放内存，释放之前必须先调用destroy
	 alloc.deallocate(p,5);
	 allocator<int> alloci;
	 vector<int> v{1,2,3};
	 auto e = alloci.allocate(v.size()*2);
	 // 拷贝vector的元素到动态内存中，返回指向最后一个元素的后一个位置的指针
	 auto g = uninitialized_copy(v.begin(),v.end(),e);
	 // 剩余元素填充为43，从g开始，填充size长度
	 uninitialized_fill_n(g,v.size(),43);
	 return 0;
 }

/**
 * 推荐使用智能指针，不要混合使用指针和智能指针；
 * 指针：与智能指针的区别i是，只能自己手动销毁。
 */
int main2(){
	//自由空间分配的空间都是无名的，故new只能返回一个指针；若空间耗尽不能分配，抛bad_alloc异常；
	int *a = new int(1);
	// 无空间不抛异常，返回空指针，此种形式int称为定位int;
	int *p2 = new (nothrow) int;
	string s("test");
	auto p1 = new auto(s);
	// const 必须进行初始化
	const int *ca = new const int(3);
	// 指针成为空悬指针，指向一块曾经保存数据对象但现在已无效的内存的指针（仍保存着已释放的动态内存的地址）
	delete ca;
	// 解决空悬指针问题
	ca = nullptr;
	return 0;
}


/**
 * 智能指针：shard_ptr,unique_ptr,weak_ptr
 * shard：允许多个指针指向同一个对象，unique 允许一个指针指向一个对象；
 * 其会根据对象的引用计数器值来自动销毁对象（析构函数）
 * weak:不控制所指向对象生存期的指针；其指向一个由shared_ptr 管理的对象。
 */
int main1(){

	string s("123");
	// 不分配内存就使用会报错
	shared_ptr<string> p = make_shared<string>(s);
	// 普通内置指针初始化智能指针
	shared_ptr<string> p2(new string("test"));
	// 这样是错的
//	shared_ptr<string> p3 = new string("test");
	cout << p << endl;
	if(p && p->empty()){
		*p = s;
	}

	// unique指针
	unique_ptr<string> p4(new string("testunique"));
	// 错误，unique不支持拷贝
//	unique_ptr<string> p5(p4);
	unique_ptr<string> p6;
	// realse放弃指针控制权，返回指针.release()不会释放内存，且会丢失指针。将p4置为空，控制权转到p5
	unique_ptr<string> p5(p4.release());
	// 控制权从p6转到p5
	p5.reset(p6.release());
	// 错误，unique不支持赋值
	//	p6 = p4;
	weak_ptr<string> wp(p);
	// weak不能直接访问对象，要调用lock，若对象存在，则返回一个shared
	// 使用weak可以阻止用户访问一个不存在的对象
	if(shared_ptr<string> np = wp.lock()){
		cout<<*np<<endl;
	}
	return 0;
}
