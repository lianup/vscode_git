#include <iostream>
#include <vector>
#include <string>
#include <memory>
using namespace std;

/**
 *拷贝控制操作：拷贝构造函数、移动构造函数（初始化时）、移动赋值运算符、拷贝赋值函数（一个对象赋予另一个对象时）、析构函数（销毁）
 */

class Foo;
class Data;

class Data{
public:
	// 使用=default生成合成版,隐式内联
	Data() = default;
	Data(const Data&) = default;
	// 定义删除的函数，虽然声明了，但是不能使用。必须出现在函数第一次声明时.
	// 析构函数以删除，不能定义该类型的变量or释放指向该类型动态分配对象的指针。但可对该类型动态分配（new）
	// 若希望阻止拷贝，应通过delete，而不应该设置为private
	Data &operator= (const Data&) = delete;//阻止赋值

};

class Foo{
public:
	Foo();
	//拷贝构造函数的参数必须是引用，参考：https://blog.csdn.net/Hackbuteer1/article/details/6545882#commentBox
	// 自定义拷贝构造函数也要自定义赋值运算符（反之亦然），不必须析构函数
	Foo(const Foo&);// 拷贝构造函数，第一个参数必须是引用类型
	Foo& operator=(const Foo&);//赋值运算符
	Foo& operator=(Foo foo);
	// 若自定义析构函数，也需要自定义拷贝赋值运算符和拷贝构造函数，若使用合成的（简单拷贝指针成员），可能导致成员被delete两次，造成
	// 未定义的错误。
	~Foo();// 析构函数，销毁是在析构函数体之后执行的。析构函数不销毁对象。	
	void swap(Foo &l,Foo &r);
	int i;
	string *s;
};

/**
 * 分配资源、薄记工作的累需要拷贝控制；
 * 自定义swap，对于非值类型交换指针，进行优化； 
 */
void Foo::swap(Foo &l,Foo &r){
	using std::swap;
	swap(l.s,r.s);
	swap(l.i,r.i);

}

/**
 *  使用swap函数定义赋值运算符，自动就是异常安全，且能正确处理赋值。
 */
Foo& Foo::operator=(Foo foo){
	swap(*this,foo);
	return *this;
}

/**
 * 赋值运算符通常包括析构函数和构造函数的操作；销毁左侧运算符对象的资源，从右侧运算符对象拷贝数据，要保证一个对象
 * 赋予自身时仍能正确工作。
 */
Foo& Foo::operator=(const Foo &foo){
	auto newp = new string(*foo.s);
	delete s;
	s = newp;
	i = foo.i;
	return *this;
}





int main(){

	//移动构造函数特点：1.仅当一个累没有定义任何版本的拷贝控制成员，且类的每个非static成员均可移动时编译器才会合成；
	//2.移动操作永远不会隐式定义为删除的函数
	//右值引用：绑定到值的引用（左值则是绑定到对象）,其只能绑定到临时对象，故：
	//其所引用的对象将要被销毁；该对象没有其他用户。
	int i = 42;
	int &r = i;//左值引用
	int && r2 = i*11;// 两个&&表右值
	int const &r3 = i*11;//敞亮引用可绑定到一个右值上


	return 0;

}

int main2(){

	string test("10"); //直接初始化，使用普通的函数匹配来选择与我们提供的参数最符合的构造函数。
	string t2 = "10";// 拷贝初始化
	string t3(t2);// 拷贝初始化，将右侧对象拷贝到当前对象，若需要还要进行类型转换。
	vector<string> v;
	v.push_back(t2);// 拷贝初始化
	v.emplace_back("a");//直接初始化
	return 0;
}
