/* 股票对象池 */
#include <iostream>
#include <memory>
#include <string>
#include <map>
using namespace std;

class Stock
{
private:
    /* data */
public:
    Stock(/* args */);
    ~Stock();
};


class StockFactory
{
private:
    mutable MutexLock mutex_;
    map<string, shared_ptr<Stock>> stocks;
public:
    StockFactory(/* args */);
    ~StockFactory();
    shared_ptr<Stock> get(const string& key);   
};

