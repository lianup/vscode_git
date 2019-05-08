#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <set>
using namespace std;
class Quote;
class Bulk_quote;
class Last;
class Basket;


/**
 * 不可被继承的类
 */
class Last final{
};

/**
 * 表原价出售的书籍
 * 函数调用的解析过程：先查找静态类型，再查找动态类型。
 */
class Quote{
public:
	// 下面两个虚函数申请一份当前对象的拷贝
	virtual Quote* clone() const & {
		return new Quote(*this);
	}
	virtual Quote* clone() & {
		return new Quote(move(*this));
	}
	Quote() = default;
	Quote(const string& book, double sales_price) : bookNo(book), price(sales_price) {}
	string isbn() const { return bookNo;}
	// 返回书籍的实际销售价格，虚函数
	// 虚函数：派生类进行覆盖的基类函数，动态绑定。任何除构造函数外的非静态函数都可以是虚函数。虚函数在派生类中也是虚函数。
	virtual double net_price(size_t n) const { return n * price;}
	// 基类通常都应该定义一个虚析构函数,确保指针的静态类型与被删除对象的动态类型不匹配的情况下，析构函数能正确调用
	virtual ~Quote() = default;
private:
	string bookNo;
// 自己和派生类可访问
protected:
	double price = 0.0;

};

/**
 * 派生类
 * 派生类的作用于位于基类之内（查找成员派生类->直接基类->间接基类）
 * public:访问说明符，控制派生类从基类集成的成员是否对派生类的用户可见
 * 此派生是共有的，则基累的成员也是派生类接口的组成部分。
 */
class Bulk_quote : public Quote{
public:
	virtual Bulk_quote* clone() const & {
		return new Bulk_quote(*this);
	}
	virtual Bulk_quote* clone() & {
		return new Bulk_quote(move(*this));
	}
	// 新标准允许派生类用override显式地注明改写虚函数的成员函数
	Bulk_quote() = default;
	// 继承的构造函数，使用using显示使用，编译器将会生成代码;默认、拷贝、移动鼓噪函数不会被继承；
	// 若一个基类构造函数含有默认实参，实参不会被继承，派生类将会获得多个继承的构造函数，每个分别省略掉一个含有默认实参的形参；
	//using Quote::Quote();
	// 初始化基类成员要调用基类构造函数
	Bulk_quote(const string& book, double p, size_t qty, double disc) : Quote(book, p), min_qty(qty), discount(disc) { }
	// 若达到了购买书籍的某个最低限量价，就可以享受折扣价格了
	double net_price(size_t cnt) const override {
		if (cnt >= min_qty)
			return cnt * (1 - discount) * price;
		else
			return cnt * price;
	}
private:
	// 使用using，保持对象尺寸相关的成员的访问级别（即，如果是private Quote，则price这个成员应该是pirvate的，使用using可把它编程
	// private(因为它在private的访问域里面)的）
//	using Quote::price;
	size_t min_qty = 0;// 适用折扣政策的最低购买量
	double discount = 0.0;//以小数表示折扣额度
};

/**
 * 定义折扣策略
 * 含有（or未经覆盖直接继承）纯虚函数的类是抽象基类。不能创建抽象基类的对象。
 */
class Disc_quote : public Quote{
public:
	Disc_quote() = default;
	Disc_quote(const string& book, double price, size_t qty, double disc) : Quote(book, price), quantity(qty), discount(disc) { }
	// 纯虚函数，无须定义。若要提供定义，函数体必须定义在类的外部。
	double net_price(size_t) const = 0;
//  派生类的成员or友元只可以通过派生类对象来访问基类的受保护成员。
protected:
	size_t quantity = 0;
	double discount = 0;
};


/**
 * 动态绑定（运行时绑定）
 * 可输入Quote or Quote 的子类
 */
double print_total(ostream &os, const Quote &item, size_t n){
	double ret = item.net_price(n);
	os << "isbn :" << item.isbn() << " sold:" << n << " total due:" << ret << endl;
}

/**
 * 辅助类 
 */
class Basket{
public:

	// 让辅助类内部处理动态内存，用户只需传入一个Quote
	void add_item(const Quote& sale){
		items.insert(shared_ptr<Quote>(sale.clone()));
	}
	void add_item(const Quote&& sale){
		items.insert(shared_ptr<Quote>(move(sale).clone()));
	}
	void add_item(const shared_ptr<Quote> &sale){
		items.insert(sale);
	}
	// 打印每本书的总价和购物篮中所有书的总价
	double total_receipt(ostream&) const;
private:
	static bool compare(const shared_ptr<Quote> &lhs, const shared_ptr<Quote> &rhs){
		return lhs->isbn() < rhs->isbn();
	}
	// 定义了一个multiset类型的成员items， 保存多个报价，按照compare成员排序
	multiset<shared_ptr<Quote>, decltype(compare)*> items{compare};
};

double Basket::total_receipt(ostream &os) const {
	double sum = 0;
	// upper_bound返回一个迭代器，该迭代器指向这批元素（关键字相同的元素）的尾后位置
	for(auto iter = items.cbegin();iter != items.cend();iter = items.upper_bound(*iter)){
		sum += print_total(os, **iter, items.count(*iter));
	}
	os << "total sale: " << sum << endl;
	return sum;
}

int main(){
	// 用容器存放继承体系中的对象时，一般用间接存储的方式。（不允许向容器中存放多种类型对象）
	// 应该在容器中放置（智能）指针而非对象！
	vector<shared_ptr<Quote>> basket;
	// 调用push_back时该对象被转换为shared_ptr<Quote>类型
	basket.push_back(make_shared<Quote>("01233",50));
	basket.push_back(make_shared<Bulk_quote>("343", 50, 10, 0.25));
	cout << basket.back()->net_price(15) << endl;
	return 0;

}

int main1(){

	// 静态类型：变量声明or表达式生成时的类型
	Quote item;
	Bulk_quote bulk;
	// 动态类型：变量or表达式表示的内存中的对象的类型
	// 若表达式非指针or引用，则动态永远=静态类型
	Quote *p = &bulk;
	Quote &r = bulk;
	// 强行指定版本的虚函数，编译期确定
	p->Quote::net_price(42);
	// 错误，一个基类指针or引用绑定到一个派生类对象上，也不可以从基类转为派生类
//	Bulk_quote *bulkP = p;
	Quote item2(bulk);
	return 0;
}
