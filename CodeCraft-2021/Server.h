#pragma once
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

struct sServerItem {
    int totalCPU;
    int totalRAM;
    int hardCost; // hardware cost
    int energyCost; // energy Cost per day
};

struct sMyEachServer {
    string serName;
    int aIdleCPU; // 各个node 空闲资源
    int bIdleCPU;
    int aIdleRAM;
    int bIdleRAM;
};

class cServer {  
public:
    int serverTypeNum;
    unordered_map<string, sServerItem> info; // 服务器型号 -> 其他信息
    vector<sMyEachServer> myServerSet; // 当前已经购买的服务器 服务器id就是vector的index

    vector<int> newServerNum; // 每天购买的服务器种类数 用于输出
    vector<unordered_map<string, int>> buyRecord; // 每天 购买的每种服务器 购买记录 用于输出 (hash: 服务器型号->数目)

    int purchase(string serName, int iDay); // 购买服务器
};