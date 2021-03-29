﻿#pragma once
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

	std::tuple<double, double> getVarRequest();  //计算add和delete请求的方差值
};