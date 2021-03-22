#pragma once
#include "tools.h"

// 按图最短路径部署方案
void graphFit(cServer &server, cVM &VM, cRequests &request);

// 每天都构造图来实现部署
void dayGraphFit(cServer &server, cVM &VM, cRequests &request, int whichDay, 
	unordered_map<string, int> &dayWorkingVM, unordered_map<string, sEachWorkingVM> &dayDeleteVM, vector<double> &args);

// 每天分为多个子图，防止当天节点过多，1w个节点就离谱
void subGraphFit(cServer &server, cVM &VM, cRequests &request,
	int begin, int end, int whichDay, unordered_map<string, int> &dayWorkingVM, 
	unordered_map<string, sEachWorkingVM> &dayDeleteVM, vector<double> &args);

// 删除虚拟机（可能从当天新买的服务器删除，也可能是已购买的服务器中删除）
void deleteVM(cServer &server, cVM &VM, sRequestItem &requestTerm, unordered_map<string, sVmItem> &workVm,
	vector<pair<string, int>> &restID, unordered_map<string, sEachWorkingVM> &dayDeleteVM);

// 添加虚拟机（可能添加到当天购买的服务器，也可能是已购买的服务器中）
void addVM(cServer &server, cVM &VM, sRequestItem &requestTerm, vector<bool> &hasDeploy, vector<vector<sServerItem>> &transfer,
	unordered_map<string, sVmItem> &workVm, vector<pair<string, int>> &restID, int whichDay, int iTerm, int indexTerm,
	vector<double> &args);

// 更新转移矩阵（动态规划），即更新图中的边
void updateTransfer(cServer &server, unordered_map<string, sVmItem> &workVm, vector<pair<string, int>> &restID,
	vector<vector<sServerItem>> &transfer, int iTerm);

// 记录每个节点的最小值，并且记录路径
void updateMinCost(vector<pair<string, int>> &restID, vector<vector<sServerItem>> &transfer,
	vector<int> &path, vector<int> &minCost, int iTerm, int alpha, bool &isFirst);

// 全部都结束后，开始购买服务器和部署虚拟机
void buyServer(cServer &server, cVM &VM, cRequests &request, vector<pair<string, int>> &restID,
	vector<vector<sServerItem>> &transfer, vector<bool> &hasDeploy, vector<int> &path, int maxLoc, int whichDay, int begin);