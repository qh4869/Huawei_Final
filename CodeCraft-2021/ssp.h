#pragma once
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include <tuple>
#include <unordered_set>
#include <queue>
using namespace std;

struct sVmNode {
	string vmID;
	string vmName;
};

void ssp(cServer &server, cVM &VM, const cRequests &request);

tuple<string, queue<int>> knapSack(const cServer &server, cVM &VM, vector<pair<string, string>> &curSet, int iDay); // 遍历服务器

tuple<int, queue<int>> dp(int N, int aIdleCPU, int aIdleRAM, int bIdleCPU, int bIdleRAM, vector<pair<string, string>> &curSet, cVM &VM); // 计算每台服务器的背包问题