#include <iostream>
#include <string>
// 不要在头文件重复包含，否则报错。只要在主文件中包含即可
//#include <Graph.hpp>
using namespace std;
// 使用友元函数前需要先声明
//class Circle;
float testFriend();
class Circle{
	public:
		friend class Graph;
		Circle(float r):radis(r){};
		//  委托构造函数
		Circle(float r,float s):Circle(r){};
		Circle() = default;
		~Circle(){
			cout<<"对象消亡时自动被调用，析构函数"<<endl;
		}
		float getRadis(){
			return radis;
		}
		float getSquare(){
			return radis*pai;
		}

	private:
		float radis;
		float square;
		const float pai = 3.14;
		inline friend float testFriend(){
			return 1122;
		}

};
