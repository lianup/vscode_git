#include <iostream>
#include <string>
#include <vector>
using namespace std;

/**
 * 下次遇到转方向也可以尝试对折思路
 * waring 思路：对折
 * 先左上-右下对角线对折，再中间一列对折。
 */

void rotate(vector<vector<int>> &matrix) {
	if(matrix.empty()){
		return;
	}
	auto row = matrix.size()-1;
	auto col = matrix[0].size()-1;
	for(auto j = 0;j <= col;j++){
		for(auto i = 0;i < j;i++){
			int temp = matrix[i][j];
			matrix[i][j] = matrix[j][i];
			matrix[j][i] = temp;
		}
	}
	
	for(auto j = 0;j < matrix[0].size()/2;j++){
		for(auto i = 0;i <= row;i++){
			int temp = matrix[i][j];
			matrix[i][j] = matrix[i][col-j];
			matrix[i][col-j] = temp;
		}
	}

}

int main(){

	vector<vector<int>> matrix{{1,2,3},{4,5,6},{7,8,9}};
	rotate(matrix);
	for(int i = 0;i < matrix.size();i++){
		for(int j = 0;j < matrix[0].size();j++){
			cout << matrix[i][j] << " ";
		}
		cout << endl;
	}

	return 0;
}
