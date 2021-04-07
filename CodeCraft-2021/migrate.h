#pragma once
#include "io.h"
#include "VM.h"
#include "tools.h"
#include "ssp.h"


void migrateVM_3(cServer &server, cVM &VM, int whichDay, unordered_map<string, int> &dayWorkingVM,
	int vmTotalNum, unordered_map<int, sMyEachServer> &delSerSet, vector<double> &args);

sServerItem chooseFirstServer(cServer &server, sVmItem &requestVM, cVM &VM, int index,
	unordered_map<int, sMyEachServer> &delSerSet, unordered_set<int> &emptySer, vector<double> &args, string vmID);

void subMigrateVM(cServer &server, cVM &VM, int whichDay, unordered_map<int, sMyEachServer> &delSerSet,
	unordered_map<string, int> &dayWorkingVM, unordered_set<int> &emptySer, vector<double> &args,
	int &cntMig, int &migrateNum, double ratio);

// 一次迁移大量的虚拟机
void massMigrate(cServer &server, cVM &VM, int whichDay, unordered_map<string, int> &dayWorkingVM,
	unordered_map<int, sMyEachServer> &delSerSet, int &cntMig, int &migrateNum, vector<double> &args);

// 从非空的服务器中选择合适的
sServerItem chooseNoEmptySer(cServer &server, int needCPUa, int needCPUb, int needRAMa, int needRAMb,
	vector<double> &args, unordered_map<int, sMyEachServer> &delSerSet, int outSerID,
	unordered_set<int> outSerIDSet);

// 将一个服务器里的资源集体迁移
void migrateSer(cServer &server, cVM &VM, int outSerID, unordered_map<string, int> &dayWorkingVM,
	map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int whichDay,
	unordered_map<int, sMyEachServer> &delSerSet, int inSerID, vector<double> &args);

// 从空的服务器里找能耗最低的
sServerItem chooseEmptySer(cServer &server, int needCPUa, int needCPUb, int needRAMa, int needRAMb,
	vector<double> &args, unordered_map<int, sMyEachServer> &delSerSet, int minEnergy,
	map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder);

// 将服务器里的虚拟机一个个尝试性的往外迁移
bool migrateSerVm(cServer &server, cVM &VM, int outSerID, unordered_map<string, int> &dayWorkingVM,
	map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int whichDay, int index,
	unordered_map<int, sMyEachServer> &delSerSet, vector<double> &args, int &cntMig, int migrateNum);

// 迁移到非空服务器中
bool migToNoEmptySer(cServer &server, int &needCPUa, int &needCPUb, int &needRAMa, int &needRAMb, vector<double> &args,
	unordered_map<int, sMyEachServer> &delSerSet, int outSerID, unordered_set<int> &outSerIDSet,
	sServerItem &myServer, sServerItem &preServer, int &cntMig, cVM &VM, unordered_map<string, int> &dayWorkingVM,
	int whichDay, map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int &minEnergy, int &i);

// 迁移到空的服务器中
bool migToEmptySer(cServer &server, int &needCPUa, int &needCPUb, int &needRAMa, int &needRAMb, vector<double> &args,
	unordered_map<int, sMyEachServer> &delSerSet, int outSerID, unordered_set<int> &outSerIDSet,
	sServerItem &myServer, sServerItem &preServer, int &cntMig, cVM &VM, unordered_map<string, int> &dayWorkingVM,
	int whichDay, map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int &minEnergy, int &i, 
	bool &findNoEmpty);

// 实在是空不了了，可以考虑购买新的服务器
bool buyNewSer(cServer &server, int &needCPUa, int &needCPUb, int &needRAMa, int &needRAMb,
	unordered_set<int> &outSerIDSet, int &Cost, sServerItem &preServer, int outSerID, sServerItem &myServer,
	cVM &VM, unordered_map<string, int> &dayWorkingVM, int whichDay, vector<double> &args,
	map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder,
	unordered_map<int, sMyEachServer> &delSerSet, int &i, int &cntMig, int migrateNum);

// 找到能满足条件的服务器，并且能耗和硬件成本最低
sServerItem chooseNewSer(cServer &server, int needCPUa, int needCPUb, int needRAMa,
	int needRAMb, int Cost);

void migrateVM_4(cServer &server, cVM &VM, int whichDay, unordered_map<string, int> &dayWorkingVM,
	int vmTotalNum, unordered_map<int, sMyEachServer> &delSerSet, vector<double> &args, int &cntMig,
	int migrateNum, int loopNum);