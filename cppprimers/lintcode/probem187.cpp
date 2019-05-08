#include<iostream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

/**
 * problem 187
 * warning
 *注意：题目的意思是环绕一周，所以是从start按顺序走下去 
 * 最简单的做法：O（n²）
 */
bool canGo(vector<int> &gas, vector<int> &cost, int i){
    int totalGas = 0, len = gas.size();
    for(int j = i;j < i + len; ++j){
        totalGas += (gas[j%len] - cost[j%len]);
        if(totalGas < 0){
            return false;
        }
    }
    return true;
}

/**
 * 优化版本，第i个失败了，从第i+j+1个开始，而不用从i+1开始
 * 时间O（n），空间O（1）
 * 具体思想参考：https://www.jiuzhang.com/solution/gas-station/#tag-other
 */
int canCompleteCircuit(vector<int> &gas, vector<int> &cost){
    if(gas.empty() || cost.empty()){
        return -1;
    }

    int i = 0;
    int len = cost.size();
    while(i < len){
        int totalCost = 0;
        int j = 0;
        for(;j < len ;++j){
            totalCost += gas[(i+j)%len] - cost[(i+j)%len];
            if(totalCost < 0){
                break;
            } 
        }
        if(j == len){
            return i;
        }
        if(j + 1 + i >= len){
            return -1;
        }else{
            // 优化，从j+1开始
            i = i + j + 1;
        }
    }

}

int canCompleteCircuit(vector<int> &gas, vector<int> &cost) {
    if(gas.empty() || cost.empty()){
        return -1;
    }
    for(int i = 0;i < gas.size();++i){
        if(gas[i] >= cost[i]){
            if(canGo(gas, cost, i)){
                return i;
            }
        }
    }
    return -1;    
}



int main(){
    vector<int> gas{1,2,3,3};
    vector<int> cost{2,1,5,1};
    cout << canCompleteCircuit(gas, cost) << endl;
    return 0;
}