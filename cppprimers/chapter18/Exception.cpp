#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <new>
using namespace std;
template <typename T> class Blob{
public:
	typedef T value_type;
	//  使用作用于运算符（::）为了让编译器知道我们使用的是类型，要在前面显示加上typename
	typedef typename vector<T>::size_type size_type;
	Blob() : data(make_shared<vector<T>>()) { }
	Blob(initializer_list<T> i1) : data(make_shared<vector<T>>(i1)) { }
	size_type size() const {
		return data->size();
	}
	bool empty() const {
		return data->empty();
	}
	void push_back(const T &t){
		data->push_back(t);
	}
	void push_back(T &&t){
		data->push_back(move(t));
	}
	void pop_back();
	// 元素访问
	T& back();
	T& operator[](size_type i);

private:
	shared_ptr<vector<T>> data;
	void check(size_type i, const string &msg) const;
};

/**
 * 处理构造函数初始值跑抛出的异常：把构造函数写成函数try块
 */
template <typename T>
Blob<T>::Blob(initializer_list<T> i1) try :
data(make_shared<vector<T>>(i1)){
    
}catch(const bad_alloc &e){
    handle_out_of_memory(e);
}

/**
 * noexcept：不抛出异常
 * 场合：1.确定程序不会抛出异常；2.不知道该如何处理异常
 * 注意：编译器不会在编译时检查noexcept
 * 若注明noexcept但却又抛出了异常，则程序会被终止。
 */
//void test(int) throw(); 等价与noexcept 
void test(int) noexcept{
    // 违反异常说明
 //   throw exception();
}

void alloc(int){

}

/**
 *虚函数与异常说明：若一个虚函数承诺不会抛出异常，则派生的虚函数也该如此。
 *若基类的虚函数允许跑出异常，则派生类不作限制。
 */
class Base{
public:
    virtual double f1(double) noexcept;
    virtual int f3();
};

class Derived : public Base{
public:
    double f1(double) noexcept;
    int f3() noexcept;

};

int main(){
    // 返回ture（声明了noexcept且无throw）、false
    cout << noexcept(test(1)) << endl;

    /**
     * 异常说明与指针：函数指针和该指针指向的函数必须具有一致的异常说明。
     */
    void (*pf1)(int) noexcept = test;
    void (*pf2)(int) = test;
    //pf1 = alloc; 错误，因为alloc可能会抛出异常
    pf2 = alloc;


    return 0;
}