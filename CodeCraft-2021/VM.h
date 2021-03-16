#pragma once
#include <unordered_map>
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
	int indexTerm;
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
    unordered_map<string, sVmItem> info; // hash map: type->info
    unordered_map<string, sEachWorkingVM> workingVmSet; // 工作中的虚拟机 哈希表： 虚拟机ID -> 该虚拟机信息
    unordered_map<string ,sPreDeployItem> preDeploy; // 预部署 vmID->其他
    vector<string> preDel; // 预删除 vmID

    vector<vector<sTransVmItem>> transVmRecord; // 每天 每条 迁移记录 用于输出
    vector<vector<sDeployItem>> deployRecord; // 每天 每条 虚拟机部署记录 用于输出
	vector<vector<sDeployItem>> realDeployRecord; // 每天 每条 虚拟机部署记录 用于输出

    // 单节点预部署
    void predp(string vmID, bool isNew, int serID, string vmName, cServer &server, bool node);
    // 双节点预部署
    void predp(string vmID, bool isNew, int serID, string vmName, cServer &server);

    // 预删除虚拟机
    void preDltVM(cServer &server, string vmID);

    // 后部署
    int postDpWithDel(cServer &server, const cRequests &request, int iDay);

    // 单节点部署
    int deploy(cServer &server, int iDay, string VMid, string vmName, int serID, bool node);
    // 双节点部署
    int deploy(cServer &server, int iDay, string VMid, string vmName, int serID);

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

	// CYT
	int deploy(cServer &server, int iDay, string VMid, string vmName, int serID, bool node, int indexTerm);
	int deploy(cServer &server, int iDay, string VMid, string vmName, int serID, int indexTerm);
};
