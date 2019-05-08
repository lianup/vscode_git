#include <iostream>
#include <vector>
#include <string>
using namespace std;

vector<int> spiralOrder(vector<vector<int>> &matrix) {
	vector<int> res;
	if(matrix.size() == 0){
		return res;
	}
	int boundI1 = matrix.size()-1,boundJ1 = matrix[0].size()-1;
	int i = 0,j = 0;
	auto size = matrix.size()*matrix[0].size();
	res.push_back(matrix[0][0]);
	while(true){
		while(j < boundJ1){
			++j;
			res.push_back(matrix[i][j]);
		if(res.size() == size){
			break;
		}
		}
		while(i < boundI1){
			++i;
			res.push_back(matrix[i][j]);
			
		if(res.size() == size){
			break;
		}
		}
		int tempJ = j;
		while(j > boundJ1 - tempJ ){
			--j;
			res.push_back(matrix[i][j]);

		if(res.size() == size){
			break;
		}
		}
	
		int tempI = i;
		while(i > boundI1 - tempI ){
			--i;
			res.push_back(matrix[i][j]);
			if(res.size() == size){
				break;
			}
		}
		--boundI1;
		--boundJ1;
	
	}
    }


int main(){
	vector<vector<int>> ve{{1,2,3,4},{5,6,7,8},{9,10,11,12},{13,14,15,16}};
	vector<int> res = spiralOrder(ve);
	for(auto it = res.begin();it != res.end();it++){
		cout<<*it<<endl;
	}
	return 0;
}
