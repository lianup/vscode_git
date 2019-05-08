#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

vector<vector<int>> res;
void helper(vector<int> &candidates, int target, int start, vector<int> &out){
    if(target == 0){
        res.push_back(out);
    }
    for(int i = start;i < candidates.size();++i){
        if(target >= candidates[i]){
        out.push_back(candidates[i]);
        helper(candidates, target - candidates[i], i, out);
        out.pop_back();
        }else{
            break;
        }
        
    }
}



/**
 * warning
 * 组合问题 dfs
 * 关键：去重手段：排个序，从当前的数开始选择，并且去除重复的数字
 */
vector<vector<int>> combinationSum(vector<int> &candidates, int target) {
    if(candidates.empty()){
        return res;
    }
    vector<int> out;
    sort(candidates.begin(), candidates.end());
    candidates.erase(unique(candidates.begin(), candidates.end()), candidates.end()); 
    // 不用for循环，因为helper已经考虑了所有可能了
    // for(int i = 0; i < candidates.size();++i){       
        helper(candidates, target, 0, out);
    // }
    return res;
}


int main(){

    vector<int> test{2,2,3};
    int target = 5;
    vector<vector<int>> res = combinationSum(test, target);
    return 0;

}