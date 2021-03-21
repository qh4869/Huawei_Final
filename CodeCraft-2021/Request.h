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
    double varAdd; //Add请求的方差
    double varDel; //Delete请求的方差
    vector<int> numEachDay; // 每天的请求数量
    vector<vector<sRequestItem>> info; // 某一天的 某条 请求
    vector<int> numAddEachDay; // 每天的add请求数量
    vector<int> numDelEachDay; // 每天的delete请求数量

        //计算每天的add/delete请求数量, 
    void getNumEachDay();

    //计算Add请求的方差
    void getVarAdd();

    //计算delete请求的方差
    void getVarDel();
};