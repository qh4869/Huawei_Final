#pragma once
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include <tuple>
#include <unordered_set>
#include <queue>
#include <omp.h>
#include <algorithm>
#include <bits/stdc++.h>
using namespace std;

#ifdef LOCAL
#include <time.h>
#include <fstream>
#include <numeric>
extern clock_t TIMEstart, TIMEend;
#endif

struct sVmNode {
	string vmID;
	string vmName;
};

namespace cyt {
	struct sServerItem {
		int totalCPU;
		int totalRAM;
		int hardCost; // hardware cost
		int energyCost; // energy Cost per day
		string serName;   // 服务器类型
		int buyID;   // 用于记录已购买服务器的ID
		bool node;   // 判断是a节点部署还是b节点部署，true : a节点
	};
}

void ssp(cServer &server, cVM &VM, cRequests &request);

void dailyPurchaseDeploy(cServer &server, cVM &VM, cRequests &request, int iDay);

void packAndDeploy(cServer &server, cVM &VM, vector<pair<string, string>> &curSet, int iDay);

tuple<string, queue<int>> knapSack(const cServer &server, cVM &VM, vector<pair<string, string>> &curSet, int iDay); // 遍历服务器

tuple<int, queue<int>> dp(int N, int aIdleCPU, int aIdleRAM, int bIdleCPU, int bIdleRAM, vector<pair<string, string>> &curSet, cVM &VM); // 计算每台服务器的背包问题

/*modified best fit*/
cyt::sServerItem bestFit(cServer &server, sVmItem &requestVM);
int srchInMyServerDouble(cServer &server, sVmItem &requestVM);
std::tuple<int, bool> srchInMyServerSingle(cServer &server, sVmItem &requestVM);