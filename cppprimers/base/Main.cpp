#include <iostream>
#include <vector>
using namespace std;

int get_sum(int a,int b){
	return (a+b);
}

void swap(int &a,int &b){
	int temp = a;
	a = b;
	b = temp;
}

void swap2(int *a,int *b){
	int temp = *a;
	*a = *b;
	*b = temp;
}

int main()
{
	int a = 5,c=10;
	double b = 5.0;
	const int *p = &a;
	vector<int> arr(10);
	int *q = const_cast<int*>(p);
	int res = static_cast<int>(a*b);
//       	decltyp0e0000000000(a) sum = test;
	int get = 0;
	for(int &cur : arr){
		get+=cur;
	}
	int &ref_a = a,&ref_c = c;
	int *point_a = &a,*point_c = &c;
//	swap(ref_a,ref_c);
	swap2(point_a,point_c);
	cout<<a<<endl;
	return 0;
}
                    
  
