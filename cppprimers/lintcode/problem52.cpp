#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;
/**
 * 与上一题类似，都是从后向前找，唯一不同是它是找上升点
 */
vector<int> nextPermutation(vector<int> &nums) {
	vector<int> res;
	if(nums.empty()){
		return res;
	}
	int small = -1;
	for(int i = nums.size()-2;i >= 0;--i){
		if(nums[i] < nums[i+1]){
			small = i;
			break;
		}
	}
	cout << small << endl;
	res = nums;
	if(small == -1){
		reverse(res.begin(), res.end());
	}else{
		for(int i = res.size()-1;i >= 0;--i){
			if(res[i] > res[small]){
			swap(res[i], res[small]);
				break;
			}
		}
 		reverse(res.begin() + small + 1, res.end());
	}
	return res;
}
