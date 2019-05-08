#include <iostream>
#include <string>
#include <fstream>
#include <assert.h>
#include <sstream>
using namespace std;

/**
 *测试 sstringstream 其用于从string中存取数据
 */
int main(){
	string line;
	ifstream input("test.txt");
	string first,second;
	while(input>>line){
		istringstream record(line);
		record>>first;
		record>>second;
		cout<<first<<" "<<second;
	}
	return 0;
}

/**
 * 测试ofstream
 */
int main3(){

	ofstream output;
	output.open("test.txt",ofstream::app);
	if(!output){
		cerr<<"wrong"<<endl;
	}
	output<<"my output"<<endl;
	return 0;
}

/**
 * 测试ifstream
 */
int main2(){
// 绝对路径
//	ifstream input("/home/lianup/cpp/io/test.txt");
	ifstream input("test.txt");
	if(!input){
		cerr<<"wrong"<<endl;
	}
	string get;
	while(input>>get){
		cout<<get<<endl;
	}
	return 0;
}

/**
 * 测试 stream
 */
int main1(){

	int a;
	// 当前cin的状态
	auto state = cin.rdstate();
	cin>>a;
	cout<<state<<" "<<a<<endl;
	cout<<a<<endl;
	return 0;
}
