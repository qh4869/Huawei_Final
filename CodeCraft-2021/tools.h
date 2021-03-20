#pragma once
#include "io.h"
#include "VM.h"

// 两个版本的isFitServer，有一个没用到好像
bool isFitServer(sMyEachServer &myServer, vector<sVmItem> &vmSet);
bool isFitServer(sServerItem &serverItem, vector<sVmItem> &vmSet);

// 给单个VM选择最优的服务器
sServerItem chooseServer(cServer &server, sVmItem &requestVM);

// 从已经购买的服务器中选择最优的服务器
sServerItem chooseBuyServer(cServer &server, sVmItem &requestVM);

// 迁移使用的服务器选择函数
sServerItem chooseBuyServer(cServer &server, sVmItem &requestVM, int index);

// 看看几个连续的VM是否能放入一台服务其中，begin:表示从哪个节点开始的连续节点
sServerItem chooseServer(cServer &server, unordered_map<string, sVmItem> &workVm,
	vector<pair<string, int>> &restID, int begin);

// 从图中选择最短路径后部署虚拟机
void deployVM(cServer &server, cVM &VM, const cRequests &request, int index, int begin, int end, vector<bool> &hasDeploy,
	int whichDay, int baseNum);

// 迁移函数
void migrateVM(cServer &server, cVM &VM, int whichDay, unordered_map<string, int> &dayWorkingVM, int vmTotalNum);

/////////////////////// 下面这几个函数还没改完bug
bool mycomp(pair<int, int> i, pair<int, int> j);

void updateRequestOrder(cRequests &request, vector<pair<int, int>> &order, int whichDay, int begin);

void requestOrder(cVM &VM, cRequests &request);