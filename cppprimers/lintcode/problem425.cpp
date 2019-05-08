#include <iostream>
#include <string>
#include <vector>
using namespace std;
//class Solution;
//vector<string> letterCombinations(string &digits);

//class Solution{
//public:
vector<string> res;
/**
 * warning : 参考https://www.jiuzhang.com/solution/letter-combinations-of-a-phone-number/#tag-highlight-lang-cpp
 */
void dfs(int curL,int length,string s,string digits,char arr[][4]){
	if(curL == length){
		res.push_back(s);
		return;
	}
	int num = digits[curL] - '0';
	for(int i = 0;i < 4;i++){
		if(arr[num][i]){
			dfs(curL+1,length,s+arr[num][i],digits,arr);
		}
	}
}
vector<string> letterCombinations(string &digits) {
        // write your code here
        if(digits.length() == 0){
            return res;
        }
	char arr[10][4]{{' '},{},{'a','b','c'},{'d','e','f'},{'g','h','i'},{'j','k','l'}
	,{'m','n','o'},{'p','q','r','s'},{'t','u','v'},{'w','x','y','z'}};
	dfs(0,digits.size(),"",digits,arr);
	return res;	
    }
//};
int main(){
	string str = "23";
	vector<string> get = letterCombinations(str);
	for(auto it = get.begin();it < get.end();++it){
		cout<<*it<<endl;
	}
	return 0;
}
