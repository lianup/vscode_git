#include <iostream>
#include <string>
#include <vector>
using namespace std;

/**
 * 时间复杂度O(logn)
 * 参考：https://www.jiuzhang.com/solution/divide-two-integers/#tag-highlight-lang-cpp
 */
int divide2(int dividend, int divisor) {
        // write your code here
        // 注意这里判断条件怎么写
    int isNeg = (dividend > 0) == (divisor > 0) ? 1 : -1;
    if(!dividend){
        return 0;
    }
    if(!divisor){
        return 2147483647;
    }
    
    long long a = llabs(dividend);
    long long b = llabs(divisor);
    // 注意一定要为long long，注意溢出情况
	long long count = 0,res= 0;
	while(a>=b){
	for(long long c = b,count=1;a >= c;c<<=1,count<<=1){
	    a-=c;
	    res+=count;
	   // cout<<res<<endl;
	}
	}
	// 注意溢出情况处理
    if (res == 2147483648 && isNeg > 0){
        return 2147483647;
    }
	return isNeg*res;
    }

/**
 * 此种做法会超时，但思路是对的，就是用除数加or用被除数减，
 * 但要用logn优化
 */
int divide(int dividend, int divisor) {
        // write your code here
	if(divisor > dividend){
		return 0;
	}
	int res = 0;
	int count = 0;
	while(count < dividend){
		count+=divisor;
		res++;
	}
	return res;
    }

int main(){
	cout << divide(10,5) << endl;
	return 0;
}

