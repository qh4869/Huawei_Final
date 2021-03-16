#pragma once
#include "io.h"
#include "Server.h"
#include "VM.h"

sServerItem chooseServer(cServer &server, sVmItem &requestVM);

sServerItem chooseServer(cServer &server, unordered_map<string, sVmItem> &workVm, vector<pair<string, int>> &restID, int begin);

bool isFitServer(sMyEachServer &myServer, vector<sVmItem> &vmSet);

bool isFitServer(sServerItem &serverItem, vector<sVmItem> &vmSet);

void deployVM(cServer &server, cVM &VM, const cRequests &request, int index, int begin, int end, vector<bool> &hasDeploy,
	int whichDay, int baseNum);

void deployServerID(cServer &server, int whichDay);

void deployRecord(cServer &server, cVM &VM, const cRequests &request, int whichDay);