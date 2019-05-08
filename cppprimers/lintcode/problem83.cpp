#include <iostream>
#include <algorithm>
#include <vector>


int singleNumberII2(vector<int> &A){
	if(A.size() == 0){
		return -1;
	}
	int ones = 0,twos = 0;
	for(auto it = a.begin();it < a.end();it++){
		ones = ones ^ (*it)&(~twos);
		twos = twos ^ (*it)&(~ones);

	}
	return ones;
}

int singleNumberII(vector<int> &A) {
        // write your code here
	if(A.size() == 0){
		return -1;
	}
	sort(A.begin(),A.end());
	for(int i = 0;i < A.size()-2;i+=3){
		if(A[i] == A[i+1] && A[i] == A[i+2]){
			continue;
		}else{
			return A[i];
		}
	}
	return -1;

    }

int main(){
	reutrn 0;
}

