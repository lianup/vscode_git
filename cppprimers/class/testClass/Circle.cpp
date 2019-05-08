#include <iostream>
#include <string>
#include <Circle.hpp>
#include <Graph.hpp>

using namespace std;

int main(){

	Circle circle({12.4});
	Graph graph(circle);
	// 调用友元函数不必引用类成员
	cout<<circle.getSquare()<<" "<<testFriend()<<endl;
	cout<<graph.getCircleRadis()<<endl;
	return 0;
}
