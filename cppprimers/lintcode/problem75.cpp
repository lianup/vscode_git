#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;
/**
 * 我想复杂了，利用二分法，根据题干条件，如果两边都大于的话，只要随便取一边就好了
 */
int findCore(int index,vector<int> &A,bool visited[]){
	visited[index] = true; 
	if(index == 0 && !visited[1]){
		return findCore(1,A,visited);
	}
	if(index == A.size() - 1 && !visited[index-1]){
		return findCore(index-1,A,visited);
	}
	if(A[index-1] < A[index] && A[index+1] < A[index]){
		return index;
	}
	if(!visited[index-1] && !visited[index+1]){
		return findCore(index-1,A,visited) || findCore(index+1,A,visited);
	}else if(!visited[index-1]){
		return findCore(index-1,A,visited);
	}else{
		return findCore(index+1,A,visited);
	}
}

/**
 * warning
 * 正确做法：若刚好，则返回；否则返回大的一个，若两个都大，则随便返回一个（根据题干意思，此情况下两边都可以得到峰值）
 */
int findPeak2(vector<int> &A){
	// 注意取值
	int left = 1, right = A.size();
	while(left < right){
		int mid = left + (right - left)/2;
		if(A[mid] > A[mid-1] && A[mid] > A[mid+1]){
			return mid;
		}else if(A[mid - 1] > A[mid]){
			right = mid-1;
		}else{
			left = mid+1;
		}
	}
	// 没有找到的情况，但本题限制条件，所以不可能出现。
	return -1;
}

int findPeak(vector<int> &A) {
	bool visited[A.size()];
	return findCore(0,A,visited);	
}


int main(){
	vector<int> A{1,2,3,4,1};
	cout << findPeak(A) << endl;
	return 0;
}
