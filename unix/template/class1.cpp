#include <iostream>
#include <vector>
#include <queue>
#include <deque>
#include <set>
#include <map>
#include <unordered_set> /* unordered set/multiset */
#include <unordered_map> /* unordered map/multimap */
#include <iterator>
#include <numeric> /* some numeric algorithm */
#include <functional>
#include <algorithm>
using namespace std;

template<typename T>
T square( T x )
{
    return x*x;
}

int main()
{
    vector<int> vec;
    vec.push_back(5);
    vec.push_back(1);
    vec.push_back(455);
    vector<int>::iterator itr1 = vec.begin();
    vector<int>::iterator itr2 = vec.end();
    for( vector<int>::iterator itr = itr1; itr != itr2; ++itr )
    {
        cout << *itr << endl;
    }
    sort(itr1, itr2);    
    cout << square(5) << endl;
}