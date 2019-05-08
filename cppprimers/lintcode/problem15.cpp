#include <iostream>
#include <vector>
#include <string>

using namespace std;

vector<vector<int>> res;

void dfs(vector<int> &nums,int n){
	if(n == 0){
		res.push_back(nums);
		return;
	}
	/**
	 * 利用全排列的特点，就是排列组合
	 */
	for(int i = 0;i <= n;i++){
		swap(nums[i],nums[n]);
		dfs(nums,n-1);
		swap(nums[i],nums[n]);
	}
}

/**
 * 方法1：暴力dfs
 */
void dfs(int curI,vector<int> &nums,vector<int> &cur,bool visited[]){
	if(nums.size() == cur.size()){
		res.push_back(cur);
		return;
	}
	visited[curI] = true;
	for(int i = 0;i < nums.size();++i){
		if(i != curI && !visited[i]){
			cur.push_back(nums[i]);
			dfs(i,nums,cur,visited);
			cur.pop_back();
			// 注意要及时设置为false才可以
			visited[i] = false;
		}
	}
//	visited[curI] = true;
}

vector<vector<int>> permute(vector<int> &nums) {
	if(nums.size() == 0){
		vector<int> a;
		res.push_back(a);
		return res;
	}
	for(int i = 0;i < nums.size();i++){
		bool visited[nums.size()];
		vector<int> a;
		a.push_back(nums[i]);
		dfs(i,nums,a,visited);
	}
	return res;
	  }


int main(){
	vector<int> nums{1,2,3};
	permute(nums);
	for(auto it = res.begin();it != res.end();++it){
		for(auto get = (*it).begin();get != (*it).end();++get){
			cout<<*get<<" ";
		}
		cout<<endl;
	}
	return 0;
}
