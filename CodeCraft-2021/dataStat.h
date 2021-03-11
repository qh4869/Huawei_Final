#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <iostream>
using namespace std;

struct sServerItem {
    string serName;
    int totalCPU;
    int totalRAM;
    int hardCost; // hardware cost
    int energyCost; // energy Cost per day
};

struct sVmItem { // hash map 的 value部分
    int needCPU;
    int needRAM;
    bool nodeStatus; // true表示双节点
};

struct sRequestItem {
    bool type; // operation type, true表示add false表示delete
    string vmName; // delete类型下这个值为空
    string vmID;
};

class cServer {
public:
    int serverTypeNum;
    vector<sServerItem> info;
};

class cVM {
public:
    int vmTypeNum;
    unordered_map<string, sVmItem> info; // hash map
};

class cRequests {
public:
    int dayNum; // 天数
    vector<int> numEachDay; // 每天的请求数量
    vector<vector<sRequestItem>> info; // 某一天的 某条 请求
};

// 输入数据读取
std::tuple<cServer, cVM, cRequests> dataIn(string fileName);