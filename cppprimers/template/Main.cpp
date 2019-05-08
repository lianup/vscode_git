#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <cstring>
#include <utility>
template <typename T> class Blob;
template <typename T> class BlobPtr;
template <typename T> bool operator==(const Blob<T>&, const Blob<T>&);
template <typename T> class BlobP{
friend class BlobPtr<T>;
friend bool operator==<T>(const Blob<T>&, const Blob<T>&);
// 模板类型参数声明为友元
// friend T;
};
// 使用using把twin定义为成员类型相同的pair的别名
using namespace std;
template<typename T> using twin = pair<T, T>;

/**
 * 每次调用时用户都要为T提供一个显式模板实参
 * 如：auto val3 = sum<long>(i, lng);
 */
template <typename T1, typename T2, typename T3>
T1 sum(T2, T3);

typedef double A;
// 显式实例化：避免多个文件中都有一个模板的示例而造成的开销过大,此声明外的文件必须有一个是非extern的定义.
// 实例化定义会实例化所有成员
//extern template class Blob<string>;
//template Blob<string>;实例化定义
/**
 * 模板内不能重用参数名
 */
template <typename A, typename B> void f(A a, B b){
	// temp的类型为参数A的类型
	A tmp = a;
	// 错误重声明模板参数B
//	double B;
}
/**
 * 默认类型为int
 * Numbers<> get （表示使用默认类型）
 */
template <class T = int> class Numbers{
public:
	// 默认值为0
	Numbers(T v = 0): val(v) { }
private:
	T val;
};


/**
 * 成员模板：一个类（无论是普通or类模板），可以包含本身是模板的成员函数。
 * 实现自己的集合类模板 
 * 默认情况下，对于一个实例化了的类模板，其成员(包括static成员)只有在使用的时候才会实例化
 */
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

template <typename T> 
void Blob<T>::check(size_type i, const string &msg) const{
	if(i > data->size()){
		throw out_of_range(msg);
	}
}

template <typename T>
T& Blob<T>::back(){
	check(0, "back on empty Blob");
	return data->back();
}
template <typename T>
T& Blob<T>::operator[](size_type i){
	check(i, "subscript out of range");
	return (*data)[i];
}

template <typename T>
void Blob<T>::pop_back(){
	check(0, "subscript out of range");
	data->pop_back();
} 

/**
 * 若试图访问一个不存在的元素，其会抛异常
 */
template <typename T> class BlobPtr{
public:
	BlobPtr(): curr(0) { }
	BlobPtr(Blob<T> &a, size_t sz = 0): wptr(a.data), curr(sz) { }
	T& operator*() const{
		auto p = check(curr, "dereference past end");
		return (*p)[curr];
	}
	// 等于BlobPtr<T>& operator++;当我们处在类的作用于中，就可以这样写
	BlobPtr& operator++();
	BlobPtr& operator--();
	BlobPtr operator++(int);
private:
	shared_ptr<vector<T>> check(size_t, const string&) const;
	// 保存一个weak_ptr,表示底层vector可能被销毁
	weak_ptr<vector<T>> wptr;
	size_t curr;
};

template <typename T> 
BlobPtr<T> BlobPtr<T>::operator++(int){
	BlobPtr ret = *this;
	++*this;
	return ret;
}

/**
 * 函数模板`
 * 模板定义:<>（此为模板参数列表，不能为空）
 * typename == class 
 * 函数模板也可以定义为inline和constexpr，说明符放在模板参数列表之后，返回类型之前
 * 编写泛型代码的两个重要准则：1.模板中的函数是const的引用；2.函数体中的条件判断仅使用<
 */
template <typename T>
int compare(const T &v1, const T &v2){
	if(v1 < v2) return -1;
	if(v2 < v1) return 1;
	return 0;
}

/**
 * <>中定义非类型参数，其表示一个值
 * 非类型模板参数的模板实参必须是常量表达式
 */
template<unsigned N, unsigned M>
int compare(const char (&p1) [N], const char (&p2) [M]){
	return strcmp(p1, p2);
}

/**
 * 模板特例化：其本质是实例化一个模板，而非重载。因此其不影响函数匹配。
 * 空<>表示我们为原模板的每个模板参数都提供实参。
 * 原模板为L：template <typename T> int compare(const T&, const T&);
 * compare的特殊版本，处理字符数组的指针
 */
template <>
int compare(const char* const &p1, const char* const &p2){
	return strcmp(p1, p2);
}

/**
 * 使用模板类
 */
int main(){
	twin<string> authors;
	Blob<int> squares = {1,2,3,3};
	for(size_t i = 0;i != squares.size();++i){
		squares[i] = i*i;
	}
	return 0;
}

int main1(){
	// 实例化出函数模板 int compare(const int&, const int&)
	cout << compare(1,0) << endl; 
	vector<int> vec1{1,2,3};
	vector<int> vec2{3,4,5};
	cout << compare(vec1, vec2) << endl;
	cout << compare("hi1", "test") << endl;
	return 0;
}
