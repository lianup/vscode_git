#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <bitset>

using namespace std;
/**
 * 其为一个类array的类模板，具有固定大小，定义时需申明含有多少个二进制位
 */

/**
 * bitset 操作
 */
int main(){
	bitset<32> bitvec(1U); // 32位，低位为1，高位为0
	bool is_set = bitvec.any();// true，返回是否存在置位的二进制位
	bool is_not_set = bitvec.none();// false
	bool all_set = bitvec.all();// false
	size_t onBits = bitvec.count(); // 1，置位的位数
	size_t sz = bitvec.size();
	bitvec.flip();// 翻转位
	bitvec.flip(0);:
	bitvec.reset();// 复位
	bitvec.reset(4);
	bitvec.set(); // 置位
	bitvec.set(bitvec.size()-1);

	bitvec[0] = 0;
	bitvec[0].flip();
	~bitvec[0];

	// 提取bitset的值
	unsigned long ulong = bitvec.to_ulong();
	cout << "ulong = " << ulong << endl;

	// IO运算符 
	bitset<16> bits;
	cin >> bits; // 遇到不是0 1的字符停止
	cout << "bits:" << bits << endl;
	return 0;
}

/**
 * 初始化一个bitset时，给定的整形值size会被转化为unsigned long long并被当做位模式来处理.
 * 若size>unsigned long long ,则剩余的高位被置位为0；若小于，则只使用给定值中的低位，高位丢弃
 */
int main1(){

	// 13位，0xbeef = 1111011101111,比初始值小，则高位被丢弃
	bitset<13> bitvec1(0xbeef);
	// 20位，比初始值大，则高位置位为0
	bitset<20> bitvec2(0xbeef);

	// 字符串初始化bitset，直接被当成二进制
	bitset<32> bitvec4("1100");
	string str("11111110000000000110001101");
	// 从 str[5] 开始的四个二进制位
	bitset<32> bitvec5(str, 5, 4);
	// 使用最后四个字符
	bitset<32> bitvec6(str, str.size() -4);
	return 0;
}
