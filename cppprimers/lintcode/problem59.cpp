#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <limits.h>
using namespace std;
/**
 * warning
 *思路类似3sum，但条件成了要diff的差值
 */
int threeSumClosest(vector<int> &numbers, int target) {
	if(numbers.size() <= 2){
		return -1;
	}
	sort(numbers.begin(), numbers.end());
	int minDiff = INT_MAX;
	int ans = 0;
	for(int i = 0;i < numbers.size()-2;i++){
		int start = i+1, end = numbers.size()-1;
		while(start < end){
			int diff = numbers[i] + numbers[start] + numbers[end] - target;
			// 如果现在的差值比之前小，则更新结果
			if(abs(diff) < minDiff){
				minDiff = abs(diff);
				ans = diff + target;
			}
			// 如果差值比0小，则说明要加一个大的数，所以++start
			if(diff < 0){
				++start;
			}else if(diff == 0){
				return target;
			}else{
				--end;
			}
		}
	}
	return ans;
}
int main(){
	vector<int> a{-1,2,1,-4};
	int target = 1;
	cout << threeSumClosest(a,target) << endl;
	return 0;
}
