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
	double varAdd; //Add请求的方差
	double varDel; //Delete请求的方差
	vector<int> numEachDay; // 每天的请求数量
	vector<vector<sRequestItem>> info; // 某一天的 某条 请求
	vector<int> numAddEachDay; // 每天的add请求数量
	vector<int> numDelEachDay; // 每天的delete请求数量

	std::tuple<double, double> getVarRequest();  //计算add和delete请求的方差值

	/*部分天数信息读取*/
	int dayReadable; // 可读取数据的天数 [K]
	int toDay = 0; // 当前已经读到第几天
	bool readOK = false; // 标记已经读完所有天数的数据
};