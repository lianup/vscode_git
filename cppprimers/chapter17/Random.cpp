#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <ctime>
using namespace std;


/**
 * 生成100个数
 */
vector<unsigned> randVec(){

	// 错误，每次得到的都是同样的100个数
//	default_random_engine e;
//	uniform_int_distribution<unsigned> u(0,9);
//	定义为static，让其保持状态
	static default_random_engine e;
	static uniform_int_distribution<unsigned> u(0,9);
	vector<unsigned> ret;
	for(size_t i = 0;i < 100; ++i){
		ret.push_back(u(e));
	}
	return ret;
}

int main(){
	default_random_engine e; // 生成无符号随机整数
	// 0-1（包含）的均匀分布
	// 分布类型都是模板，生成整数值默认为int，浮点数默认为double
	uniform_int_distribution<double> u(0,1); 
	for(size_t i = 0; i < 10; ++i){
		cout << u(e) << endl;
	}
	// 非均匀分布，均值4，标准差1.5
	normal_distribution<> n(4,1.5);
	vector<unsigned> vals(9);
	for(size_t i = 0; i != 200; ++i){
		unsigned v = lround(n(e)); // 舍入到最接近的整数
		if(v < vals.size()){
			++vals[v];
		}
	}

	// 此分布是一个普通类，返回一个bool值，几率为1/1
	bernoulli_distribution b; 
	bernoulli_distribution a(.55); // 也可以选择自己调整概率
	return 0;
}


/**
 *  种子值：引擎利用其从序列中一个新位置重新产生随机数，达到
 * 每次都会产生不同随机结果的目的。
 */
int main2(){
	// e1e2种子不同，故生成不同的序列
	default_random_engine e1;// 使用默认种子
	default_random_engine e2(22343435); // 使用给定的种子值
	default_random_engine e3;
	// e3 e4种子想相同，故生成相同的序列
	e3.seed(3232); // 设置种子值
	default_random_engine e4(3232);
	for(size_t i = 0; i != 100; ++i){
		if(e1() == e2()){
			cout << "unseede match at iteration:" << i << endl;
		}
		if(e3 != e4){
			cout << "seeded differs at iteration:" << i << endl;
		}

	}
	return 0;
}

/**
 *  随机数引擎：函数对象类
 */
int main1(){

	default_random_engine e;
	for (size_t i = 0;i < 10;++i){
		cout << e() << " ";
	}

	// 生成0 - 9 之间的（包含）均匀分布的随机数
	// 分布类型的独享
	uniform_int_distribution<unsigned> u(0,9);
	for(size_t i = 0;i < 10;++i){
		// 随机数发生器：分布对象和引擎对象的组合
		cout << u(e) << " ";
	}
	return 0;
}
