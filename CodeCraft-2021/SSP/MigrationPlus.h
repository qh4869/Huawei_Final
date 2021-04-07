#pragma once
#include "io.h"
#include "VM.h"
// #include "tools.h"
#include "ssp.h"

void massMigrate(cSSP_Mig_Server &server, cSSP_Mig_VM &VM, int whichDay, unordered_map<string, int> &dayWorkingVM,
	unordered_map<int, sMyEachServer> &delSerSet, int &cntMig, int &migrateNum, vector<double> &args);

void deleteEmpVmTar(cSSP_Mig_Server &server, map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder,
	int serID);

void updateEmpVmTarOrder(cSSP_Mig_Server &server, map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder,
	int serID);

// 将一个服务器里的资源集体迁移
void migrateSer(cSSP_Mig_Server &server, cSSP_Mig_VM &VM, int outSerID, unordered_map<string, int> &dayWorkingVM,
	map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int whichDay,
	unordered_map<int, sMyEachServer> &delSerSet, int inSerID, vector<double> &args);

// 将服务器里的虚拟机一个个尝试性的往外迁移
bool migrateSerVm(cSSP_Mig_Server &server, cSSP_Mig_VM &VM, int outSerID, unordered_map<string, int> &dayWorkingVM,
	map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int whichDay, int index,
	unordered_map<int, sMyEachServer> &delSerSet, vector<double> &args, int &cntMig, int migrateNum);

// 迁移到非空服务器中
bool migToNoEmptySer(cSSP_Mig_Server &server, int &needCPUa, int &needCPUb, int &needRAMa, int &needRAMb, vector<double> &args,
	unordered_map<int, sMyEachServer> &delSerSet, int outSerID, unordered_set<int> &outSerIDSet,
	cyt::sServerItem &myServer, cyt::sServerItem &preServer, int &cntMig, cSSP_Mig_VM &VM, unordered_map<string, int> &dayWorkingVM,
	int whichDay, map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int &minEnergy, int &i);

// 迁移到空的服务器中
bool migToEmptySer(cSSP_Mig_Server &server, int &needCPUa, int &needCPUb, int &needRAMa, int &needRAMb, vector<double> &args,
	unordered_map<int, sMyEachServer> &delSerSet, int outSerID, unordered_set<int> &outSerIDSet,
	cyt::sServerItem &myServer, cyt::sServerItem &preServer, int &cntMig, cSSP_Mig_VM &VM, unordered_map<string, int> &dayWorkingVM,
	int whichDay, map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int &minEnergy, int &i, 
	bool &findNoEmpty);

cyt::sServerItem chooseFirstServer(cSSP_Mig_Server &server, sVmItem &requestVM, cSSP_Mig_VM &VM, int index,
	unordered_map<int, sMyEachServer> &delSerSet, unordered_set<int> &emptySer, vector<double> &args, string vmID);

// 从空的服务器里找能耗最低的
cyt::sServerItem chooseEmptySer(cSSP_Mig_Server &server, int needCPUa, int needCPUb, int needRAMa, int needRAMb,
	vector<double> &args, unordered_map<int, sMyEachServer> &delSerSet, int minEnergy,
	map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder);

// 从非空的服务器中选择合适的
cyt::sServerItem chooseNoEmptySer(cSSP_Mig_Server &server, int needCPUa, int needCPUb, int needRAMa, int needRAMb,
	vector<double> &args, unordered_map<int, sMyEachServer> &delSerSet, int outSerID,
	unordered_set<int> outSerIDSet);