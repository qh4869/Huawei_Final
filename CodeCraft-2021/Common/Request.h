#pragma once
#include <string>
#include <vector>
#include "globalHeader.h"
#include <iostream>
#include <tuple>
using namespace std;

struct sRequestItem {
	bool type; // operation type, true表示add false表示delete
	string vmName; // delete类型下这个值为空
	string vmID;
	/****** 决赛新增内容 ******/
	int lifetime;   // 虚拟机的寿命
	int quote;    // 用户报价
};

class cRequests {
public:
	int dayNum; // 天数
	vector<int> numEachDay; // 每天的请求数量
	vector<vector<sRequestItem>> info; // 某一天的 某条 请求

	/*部分天数信息读取*/
	int dayReadable; // 可读取数据的天数 [K]
	int toDay = 0; // 当前已经读到第几天
	bool readOK = false; // 标记已经读完所有天数的数据

};