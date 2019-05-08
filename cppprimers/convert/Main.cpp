#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <functional>
using namespace std;
class My_data;
class absInt;
class SmallInt;

class SmallInt{
	friend
	SmallInt operator+(const SmallInt&, const SmallInt&);
public:
	// int向类类型的转换 
	SmallInt(int i = 0): val(i)
	{
		if(i < 0 || i > 255)
			throw out_of_range("Bad val");
	}
	// 类型转换运算符，类的一种特殊成员函数。不指定返回类型，参数列表要为空
	// 实际少用
	// 显式的类型转换运算符，编译器不会自动执行，除非表达式被用作条件。
	explicit operator int() const {return val;}
//	operator int() const {return val;}
private:
	size_t val;


};
SmallInt operator+(const SmallInt &a, const SmallInt &b){
	return SmallInt(a.val + b.val);
}


/**
 * lambda 是函数对象
 *  定义了函数调用运算符的类：函数对象，行为像函数一样。
 */
class absInt{
public:
	int operator()(int val) const{
		return val < 0 ? -val : val;
	}
};

class My_data{
friend My_data operator+(const My_data &left, const My_data &right);
friend istream &operator>>(istream &is, My_data &data);
friend ostream &operator<<(ostream &os, const My_data &data);
public:
	int num;
	string name;
	My_data(int a):num(a),name("test"){}
	My_data(int a, string get):num(a),name(get){}
	// 定义在类内的形式
	ostream &operator<<(ostream &os){
		cout << num << " " << name << endl;
	}
	My_data& operator+=(const My_data &right){
		num += right.num;
		return *this;
	}

	bool operator==( const My_data right){
		return num == right.num &&
			name == right.name;
	}
	/**
	 * 应该把工作委托给另一个，且应该同时存在== and ！=
	 */
	bool operator!=(const My_data &right){
		return !( *this == right);
	}

};
/**
 * 多参形式
 * 不能定义在类内，因为一元运算符重载默认接受一个参数
 */
My_data operator+(const My_data &left, const My_data &right){
	My_data res = left;
	res += right;
	return res;
}

ostream &operator<<(ostream &os, const My_data &data){
	os << data.num << " " << data.name << endl;
	return os;
}

// 必须处理输入可能失败的情况
istream &operator>>(istream &is, My_data &data){
	is >> data.num >> data.name;
	if(!is){
		data = My_data(1,"test");	
	}
	return is;
}


int main(){
	SmallInt si;
	si = 4;
//	si + 3; // si->int
//	显式地请求类型转换`
	static_cast<int>(si) + 3;
	SmallInt s1, s2;
	SmallInt s3 = s1 + s2;
	// 二义性，因为可以把0变成smallint 或者 把 s3变成int型
//	int i = s3 + 0;

}

int main2(){

	// 标准库定义的函数对象，包括在头文件functional里面
	plus<int> intAdd;
	negate<int> intNegate;//对int值求反
	int sum = intAdd(19,20);
	sum = intAdd(10, intNegate(10));// 0
	vector<string *> nameTable;
	//错误，指针的地址如此用< 将产生未定义行为，因为指针地址无关系。
//	sort(nameTable.begin, nameTable.end(), [](string *a, string *b){return a < b;});
//	标准库中定义的指针的less是良好的
	sort(nameTable.begin(), nameTable.end(), less<string*>());
	return 0;
}

int main1(){

	My_data data(1,"test");
	My_data test(2,"a");
	cin >> test;
	cout << data;
	data << cout ;
	int a(5);
	absInt absObj;
	int ui = absObj(a);
	return 0;

}
