#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
/**
 * warning
 * 找规律题
 * 规律：https://www.lintcode.com/problem/previous-permutation/note/173119
 */
vector<int> previousPermuation(vector<int> &nums) {
	vector<int> res;
	if(nums.empty()){
		return res;
	}
	int re = -1;
	for(int i = nums.size()-2;i > 0;--i){
		if(nums[i] > nums[i+1]){
			re = i;
			break;
		}
	}
	if(re == -1){
		for(auto it = nums.rbegin();it != nums.rend();++it){
			res.push_back(*it);
		}
	}else{
		res = nums;
		for(int i = res.size()-1;i > re;--i){
			if(res[i] < res[re]){
				int temp = res[re];
				res[re] = res[i];
				res[i] = temp;
				break;
			}
		
		}
		reverse(res.begin()+re+1,res.end());
	}
	return res;
}

int main(){

	vector<int> numbers{1,3,2,4};
	vector<int> res(previousPermuation(numbers));
	for(auto it = res.cbegin();it != res.cend();++it){
		cout << " " << *it;
	}
	cout << endl;
	return 0;
}
