#include <iostream>
// 不要在头文件中重复包含，否则报错，在主文件中包含即可。
//#include <Circle.hpp>
using namespace std;


class Graph{
public:
	Graph(Circle c):circle(c){};
	float getCircleRadis(){
		return circle.radis;
	}
private:
	Circle circle;
};
