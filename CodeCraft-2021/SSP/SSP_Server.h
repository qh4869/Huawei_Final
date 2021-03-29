#pragma once
#include "Server.h"

class cSSP_Server : public cServer {
public:
	int ksSize; // 背包算法分组
	vector<pair<string, sServerItem>> infoV; // vector形式的info，为了写多线程
	void genInfoV();
};