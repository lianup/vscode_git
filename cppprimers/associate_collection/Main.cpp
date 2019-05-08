#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
// 包含pair
#include <utility>
using namespace std;

/**
 * 包括set 和 map
 * multi：可存储相同元素
 * unordered：无序保存
 * 一般不对关联容器使用泛型算法，因为效率低。真要使用则把它复制到一个序列。
 */

/**
 * 自定义string的排序比较函数
 */

bool compare_set(const string &a,const string &b)
{
	return (a.size() < b.size());
	
}

/**
 * 计算输入单词的出现次数
 */
bool count_word()
{
	map<string,size_t> word_count;
	string word;
	while(cin >> word){
		// 返回一个pair，pair.first为插入成功时指向元素的迭代器，second为是否成功（bool）
		auto res = word_count.insert({word,1});
		if(!res.second){
			++res.first->second;
		}
	}
}

int main(){

	/**
	 * map 基本操作
	 */
	map<string,int> word_count;
	word_count["test"] = 2;
	// 若此下标不存在，会添加元素；对map作下标操作会得到一个map type类型的返回值
	word_count["test2"] = 3;
	// 若不存在，抛异常
	cout << word_count.at("test") << endl;
	for(const auto &w : word_count)
	{
		cout << w.first << " " << w.second << endl;
	} 
	// 插入的类型要为pair
	word_count.insert({"test3",5});
	/**
	 * set 基本操作
	 */
	set<string> word{"and","test"};
	string test("tset");
	word.insert("a");
	// 若存在，返回指向该元素的指针；否则返回end()
	if(word.find(test) == word.end())
	{
		cout << test << endl;
	}
	// set 的迭代器都是const的，都只能读；set map的迭代器都是按字典序升序遍历的
	for(auto it = word.begin();it != word.end();++it)
	{
		cout << *it << endl;
	}
	// 删除操作
	word.erase("test");
	word.erase(word.begin());
	word.erase(word.begin(),word.end());
	// 传参时传两个，第二个自定义比较的函数类型，记得加*
	multiset<string,decltype(compare_set)*> mset(compare_set);
	// 返回test的个数
	cout << mset.count("test") << endl;
	// 返回类型为迭代器，指向第一个小于，大于，等于的元素的迭代器，equal返回的是pair类型的迭代器
	cout << *mset.lower_bound("te") << " " << *mset.upper_bound("test") << *mset.equal_range("test").first << endl;
	// 其成员为public
	pair<int,int> t_pair{1,2};
	cout << t_pair.first << " " << t_pair.second << endl;

		
	return 0;
}
