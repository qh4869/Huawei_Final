#pragma once
#include <unordered_map>
#include <unordered_set>
#include "Server.h"
#include <iostream>
#include "globalHeader.h"
using namespace std;

struct sVmItem { // hash map 的 value部分
	int needCPU;
	int needRAM;
	bool nodeStatus; // true表示双节点
};

struct sEachWorkingVM {
	string vmName;
	int serverID;
	bool node; // true 表示a节点 false 表示b节点
};

struct sTransVmItem {
    string vmID;
	int serverID;
	bool isSingle;
	bool node; // 双节点的虚拟机这个变量没有意义
};

struct sDeployItem {
	int serID;
	bool isSingle;
	bool node; // true a节点
};

struct sPreDeployItem {
	bool isNew; // 是否新买服务器
	int serID;
	bool node; // true: node a
	string vmName;
};

class cVM {
public:
	int vmTypeNum;
	unordered_map<string, sVmItem> info; // 每一类虚拟机信息: type->info
	unordered_map<string, sEachWorkingVM> workingVmSet; // 工作中的虚拟机 哈希表： 虚拟机ID -> 该虚拟机信息

	vector<vector<sTransVmItem>> transVmRecord; // 每天 迁移记录 用于输出 (注意迁移是有先后顺序的)
	vector<unordered_map<string, sDeployItem>> deployRecord; // 每天 虚拟机部署记录 用于输出 vmID -> 部署详情

	// 单节点部署
	void deploy(cServer &server, int iDay, string VMid, string vmName, int serID, bool node);
	// 双节点部署
	void deploy(cServer &server, int iDay, string VMid, string vmName, int serID);

	// delete VM
	void deleteVM(string vmID, cServer& server);

	// 单节点虚拟机迁移
	void transfer(cServer &server, int iDay, string VMid, int serID, bool node);
	// 双节点虚拟接迁移
	void transfer(cServer &server, int iDay, string VMid, int serID);

	// 输入虚拟机名字，返回是否是 双节点部署
	bool isDouble(string vmName);
	// 输入虚拟机名字，返回需要CPU
	int reqCPU(string vmName);
	// 输入虚拟机名字，返回需要RAM
	int reqRAM(string vmName);

	/******** 决赛新增内容 ********/
	vector<unordered_map<string, int>> quote;   // 我方给出的虚拟机报价 : vmID->price
	vector<unordered_map<string, int>> compQuote; // 对方给出的虚拟机报价 : vmID->price
	unordered_set<string> lostVmSet;   // 失去的虚拟机集合，存放vmID(记得添加头文件)
	void updateRequest(int iDay, cRequests &request);   // 根据报价结果更新request.info
	void setQuote(cRequests &request, int iDay);   // 给出我方的报价
	vector<int> winNum; // 博弈成功次数
	vector<int> lossNum; // 博弈失败次数

};