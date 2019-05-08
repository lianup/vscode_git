#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <limits.h>
using namespace std;

/**
 * warning
 * 思路：防止O（n的三次方）复杂度，用双指针做。
 * 关键：去重步骤。去了我两个小时！！最后还是看别人怎么去的。一开始是把numbers[i]当做中间数，但这样判断会有问题。
 * 然后就把start当做中间数。
 */
vector<vector<int>> threeSum(vector<int> &numbers) {
	vector<vector<int>> res;
	if(numbers.size() < 3){
		return res;
	}
	int count = -1;
	sort(numbers.begin(),numbers.end());
	for(auto i = numbers.begin();i != numbers.end();++i){
		cout << *i << " ";
	}
	cout << endl << endl;
	int start = 0;
	auto end = numbers.size()-1;
	for(int i = 0;i < numbers.size();++i){
		start = i+1;
		auto end = numbers.size() - 1;
		int sum = -numbers[i];
		// 如果等于相邻的数直接跳过，其与下面的两步去重为关键。
		if(i != 0 && numbers[i] == numbers[i-1]){
			continue;
		}
		int lastStart = INT_MIN;

		while(start < end){ 
			int temp = numbers[start] + numbers[end];
			// 如果start有相同的，直接跳过
				if(start > i + 1 &&  numbers[start-1] == numbers[start]){
					++start;
					continue;
				}		
			if(temp == sum){
				vector<int> curRes{numbers[i], numbers[start], numbers[end]};
				res.push_back(curRes);
				lastStart = numbers[start];
				++start;
				--end;
			}else if(temp < sum){
				++start;
			}else{
				--end;
			}
		}
	}
	sort(res.begin(),res.end(),[](vector<int> &a,vector<int> &b){return a[0] < b[0];});
	return res;
}

int main(){
	vector<int> numbers{-2,-3,-4,-5,-100,99,1,4,4,4,5,1,0,-1,2,3,4,5};
	vector<vector<int>> res = threeSum(numbers);
	for(int i = 0;i < res.size();i++){
		for(int j = 0;j < 3;j++){
			cout << res[i][j] << " ";
		}
		cout << endl;
		cout << res.size();
	}
	return 0;
}
