#pragma once
#include "io.h"
#include "VM.h"
#include "tools.h"


void migrateVM_3(cServer &server, cVM &VM, int whichDay, unordered_map<string, int> &dayWorkingVM,
	int vmTotalNum, unordered_map<int, sMyEachServer> &delSerSet, vector<double> &args);

sServerItem chooseFirstServer(cServer &server, sVmItem &requestVM, cVM &VM, int index,
	unordered_map<int, sMyEachServer> &delSerSet, unordered_set<int> &emptySer, vector<double> &args);


void subMigrateVM(cServer &server, cVM &VM, int whichDay, unordered_map<int, sMyEachServer> &delSerSet,
	unordered_map<string, int> &dayWorkingVM, unordered_set<int> &emptySer, vector<double> &args,
	int &count, int &migrateNum);