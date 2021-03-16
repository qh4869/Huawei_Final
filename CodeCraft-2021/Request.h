#pragma once
#include <string>
#include <vector>
#include "globalHeader.h"
using namespace std;

struct sRequestItem {
    bool type; // operation type, true表示add false表示delete
    string vmName; // delete类型下这个值为空
    string vmID;
};

class cRequests {
public:
    int dayNum; // 天数
    vector<int> numEachDay; // 每天的请求数量
    vector<vector<sRequestItem>> info; // 某一天的 某条 请求
};