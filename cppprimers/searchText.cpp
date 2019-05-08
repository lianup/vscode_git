#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <set>
using namespace std;
class QueryResult;
calss TextQuery(){
public:
	using line_no = vector<string> size_type;
	TExtQuery(ifstream &is);
	QueryResult query(const string&) const;
private:
	shared_ptr<vector<string>> file; // 输入文件
	map<string,shared_ptr<set<line_no>>> wm; // 每个单词到它所在的行号的集合的映射
}




void runQueries(ifstrema &infile){
	TextQuery tq(infile);
	while(true){
		cout << "enter word to look for, or q to quit:";
		if(!(cin >> s) || s == "q"){
			print(cout,tq.query(s)) << endl;
		}
	}
}


int main(){
	
	return 0;
}
