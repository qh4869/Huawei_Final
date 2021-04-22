#pragma once
#include "SSP_Mig_Server.h"
#include "SSP_Mig_VM.h"
#include "SSP_Mig_Request.h"
#include <tuple>
#include <unordered_set>
#include <queue>
#include <omp.h>
#include <algorithm>
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

/*-----------------cyt-----------*/
namespace cyt { // cyt分支的部分代码
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
/*每次迁移之后更新delSerSet*/
void updateDelSerSet(unordered_map<int, sMyEachServer> &delSerSet, sVmItem &requestVM, bool node, int serID, bool flag);
/*迁移之前的delSerSet初始化，放在购买部署删除部分的结尾*/
unordered_map<int, sMyEachServer> recoverDelSerSet(cSSP_Mig_Server &server, cSSP_Mig_VM &VM, unordered_map<string, sEachWorkingVM> &dayDeleteVM);
/*---------------cyt-------------*/

// subset-sum problem
void sspEachDay(int iDay, cSSP_Mig_Server &server, cSSP_Mig_VM &VM, cSSP_Mig_Request &request);

// 每天 bestFit部署 + knapSack购买+部署 + 删除
void dailyPurchaseDeploy(cSSP_Mig_Server &server, cSSP_Mig_VM &VM, cRequests &request, int iDay,
	unordered_map<int, sMyEachServer> &delSerSet, unordered_set<string> &dayWorkingVM);

/*modified best fit*/
cyt::sServerItem bestFit(cSSP_Mig_Server &server, sVmItem &requestVM);
int srchInMyServerDouble(cSSP_Mig_Server &server, sVmItem &requestVM);
std::tuple<int, bool> srchInMyServerSingle(cSSP_Mig_Server &server, sVmItem &requestVM);

// knapSack部分 购买和部署
void packAndDeploy(cSSP_Mig_Server &server, cSSP_Mig_VM &VM, vector<pair<string, string>> &curSet, int iDay);

// knapSack中 挑选服务器 遍历
tuple<string, queue<int>> knapSack(const cSSP_Mig_Server &server, cSSP_Mig_VM &VM, vector<pair<string, string>> &curSet, int iDay); // 遍历服务器

																													// knapSack 每台服务器的效益函数 递归剪枝
tuple<int, queue<int>> dp(int N, int aIdleCPU, int aIdleRAM, int bIdleCPU, int bIdleRAM,
	vector<pair<string, string>> &curSet, cSSP_Mig_VM &VM); // 计算每台服务器的背包问题



/*Mig中提高运行速度*/
map<int, map<int, map<int, map<int, vector<int>>>>>::iterator greaterEqu1(map<int, 
	map<int, map<int, map<int, vector<int>>>>> &set, int val);
map<int, map<int, map<int, vector<int>>>>::iterator greaterEqu2(map<int, map<int, 
	map<int, vector<int>>>> &set, int val);
map<int, map<int, vector<int>>>::iterator greaterEqu3(map<int, 
	map<int, vector<int>>> &set, int val);
map<int, vector<int>>::iterator greaterEqu4(map<int, vector<int>> &set, int val);