#pragma once
#include "Server.h"
#include "VM.h"
#include "tools.h"

void graphFit(cServer &server, cVM &VM, const cRequests &request);

void dayGraphFit(cServer &server, cVM &VM, const cRequests &request, int whichDay);

void subGraphFit(cServer &server, cVM &VM, const cRequests &request, int begin, int end, int whichDay);

void deleteVM(cServer &server, cVM &VM, sRequestItem &requestTerm, unordered_map<string, sVmItem> &workVm, vector<pair<string, int>> &restID);

void addVM(cServer &server, cVM &VM, sRequestItem &requestTerm, vector<bool> &hasDeploy, vector<vector<sServerItem>> &transfer, 
	unordered_map<string, sVmItem> &workVm, vector<pair<string, int>> &restID, int whichDay, int iTerm, int indexTerm);

void updateTransfer(cServer &server, unordered_map<string, sVmItem> &workVm, vector<pair<string, int>> &restID,
	vector<vector<sServerItem>> &transfer, int iTerm);

void updateMinCost(vector<pair<string, int>> &restID, vector<vector<sServerItem>> &transfer, 
	vector<int> &path, vector<int> &minCost, int iTerm, int alpha, bool &isFirst);

void buyServer(cServer &server, cVM &VM, const cRequests &request, vector<pair<string, int>> &restID, 
	vector<vector<sServerItem>> &transfer, vector<bool> &hasDeploy, vector<int> &path, int maxLoc, int whichDay, int begin);