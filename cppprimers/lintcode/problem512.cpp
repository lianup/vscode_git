#include <iostream>
#include <string>
#include <vector>
using namespace std;


/**
 * 参考： https://www.jiuzhang.com/solution/decode-ways/#tag-highlight-lang-cpp
 */
int numDecodings2(string &s){
	if(s == "" || s[0] == '0'){
		return 0;
	}
	vector<int> dp(s.size(),0);
	dp[0] = 1;
	for(int i = 1;i < s.size();++i){
		if(s[i] != '0'){
			dp[i] += dp[i-1];
		}
		if(s[i-1] != '0' && stoi(s.substr(i-1,2) <= 26){
			dp[i] += i == 1 ? 1 : dp[i-2];
		}
		if(!dp[i]){
			return 0;
		}
	}
	return dp[s.size()-1];

}

/**
 * 此算法有问题
 */
int numDecodings(string &s){
	if(s.size() == 0){
		return 0;
	}
	if(s[0] == '0'){
		return 0;
	}
	if(s.size() == 1){
		return 1;
	}
	int count = 1;
	for(int i = 0;i < s.size()-1;++i){
		if(s[i] == '1' || s[i] == '2' && s[i+1] <= '6'){
			count*=2;
		}
	}
	return count;
}



int main(){
	string s("123413");
	cout << numDecodings2(s)<<endl;
	return 0;

}
