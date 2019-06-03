#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

/* learn vector */
int main()
{
    vector<int> vec;
    vec.push_back(11);
    vec.push_back(1);

    cout << vec[1] << " " << vec.at(1) << endl;

    /* use iterator */
    for( vector<int>::iterator itr = vec.begin(); itr != vec.end(); ++ itr )
    {
        cout << *itr << endl;
    }

    /* c++ 11 */
    for( it : vec )
    {
        cout << it << endl;
    }

}