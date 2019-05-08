#include <iostream>
#include <vector>
#include <deque>
#include <list>
#include <iterator>
/**
 * 使用泛型算法需包含的两个包
 */
#include <algorithm>
#include <numeric>
using namespace std;


bool isBigger(int a,int b)
{
	return a > b;
}



/**
* 迭代器:插入、流、反向、移动迭代器。
 */
int main(){

	/**
	 * 插入迭代器
	 */
	list<int> list1{1,2,3,4};
	list<int> list2,list3;
	// 头插，返回指向新插入元素的迭代器指针
	copy(list1.cbegin(),list1.cend(),front_inserter(list2));
	// 尾插
	copy(list1.cbegin(),list1.cend(),inserter(list3,list3.begin()));

	/**
	 * 流迭代器
	 */
	istream_iterator<int> in_it(cin);
	// 尾后迭代器，指向最后一个元素的后一个位置
	istream_iterator<int> in_eof;
	vector<int> store(in_it,in_eof);
	while(in_it != in_eof){
		store.push_back(*in_it++);
	}
	ostream_iterator<int> out_it(cout);
	// 输出迭代器，可通过copy来打印vector中的元素
	copy(store.cbegin(),store.cend(),out_it);
	cout<<endl;

	/**
	 * 反向迭代器
	 */
	string line("first,middle,last");
	// 得到反向迭代后的指向第一个逗号的迭代器
	auto rcomma = find(line.crbegin(),line.crend(),',');
	// .base()让反向迭代器变回正向
	cout << string(rcomma.base(),line.cend()) << endl;

	return 0;
}

/**
 * labmda 表达式
 */
int main2(){
	// 参数列表和返回类型可以忽略，但必须包括捕获列表和函数体
	auto f = []{return 42;};
	cout << f << endl;
	vector<int> a{1,2};
	// [] 捕获列表，（）参数列表,捕获分为值和引用捕获(应避免)，此处为值捕获，值捕获在lambda表达式创建时copy值
	// 值捕获，若在参数and捕获列表之间添加mutable，则原值可被改变
	sort(a.begin(),a.begin(),[a] (int q,int p){return (q > p);});
	// 值和引用混用，捕获列表参数一为=（默认为值捕获），&默认为引用捕获（必须）
	for_each(a.begin(),a.end(),[=,&cout](int a){cout << a << endl;});
	// 显式定义lambda表达式的返回类型
	transform(a.begin(),a.end(),a.begin(),[](int p)->int{if(p > 0) return p;else return -p;});
	return 0;
}


/**
 * 泛型算法：仅对迭代器进行操作
 */
int main1(){

	vector<int> a{1,2};
	vector<int> b{1,2,3};
	/**
	 * 只读算法（不改变元素）
	 */
	// 泛型算法，查找,找到则返回结果指向它，否则返回第二个参数
	auto res = find(a.cbegin(),a.cend(),3);
	// 计算和，参数三为和的初值
	int count = accumulate(a.cbegin(),a.cend(),0);
	// 比较两个序列是否保存相同的值，需默认序列2长于等于序列1（参数三为序列2的起始）
	auto isEqual = equal(a.cbegin(),a.cend(),b.cbegin());
	cout << *res << " " << isEqual << endl;

	/**
	 * 非只读算法
	 * 修改a的序列为0，向空集合中插入不可取，需保证有足够大的空间
	 */
	fill(a.begin(),a.end(),0);
	// 插入迭代器，通过向其赋值来插入元素
	auto it = back_inserter(a);
	*it = 42;
	// 此种做法对，因为back_inserter会在其尾部插入元素
	fill_n(it,4,11);
	vector<int> c(a.size()*2);
	// copy实现内置数组的拷贝，返回第三个参数的最后一个值的后一个位置
	int a1[] = {1,2,3};
	int b1[sizeof(a1)/sizeof(*a1)];
	auto copyRes = copy(begin(a1),end(a1),b1);
	// 把所有0 改为15
	replace(a.begin(),a.end(),0,15);
	// 不改变原序列，新序列为c
	replace_copy(a.begin(),a.end(),back_inserter(c),0,15);
	
	sort(a.begin(),a.end());
	// 自定义比较条件
	sort(a.begin(),a.end(),isBigger);
	// 把不重复的元素放在前面，返回最后一个不重复元素的后面指针
	auto uit = unique(a.begin(),a.end());
	// 自定义条件下保持等长元素的字典序
	stable_sort(a.begin(),a.end(),isBigger);
	return 0;

}
