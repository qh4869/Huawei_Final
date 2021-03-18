#include "ssp.h"

void ssp(cServer &server, cVM &VM, const cRequests &request) {
/* Fn: SSP方法
*	- 关于del的地方还可以优化
*	- 双节点的问题
*	- 背包问题：动态规划，tree分支定界，遗传算法
*	- 有一些vm是可以放进已经买的服务器中的，可以改成dp 或者 排序firstFit
*	
* Update 03.18
*	- firstFit 和 knapSack 结构互相迭代
*	- del及时处理
*/
#ifdef LOCAL
	// 估计成本变量
	int engCostStas = 0; // 计算功耗成本
	int hardCostStas = 0;
	string costName;
#endif

	bool vmIsDouble;
	int serID;
	bool serNode;
	int vmReqCPU; 
	int vmReqRAM;
	int bugID;

	/* 背包算法要处理的vm add请求集合，最多server.ksSize个元素 
	 {不能直接装入现有服务器的vm插入/deploy后的元素删除} */
	vector<pair<string, string>> curSet;

	for (int iDay=0; iDay<request.dayNum; iDay++) { // 每一天的请求
#ifdef LOCAL
		cout << iDay << endl;
		TIMEend = clock();
		cout<<(double)(TIMEend-TIMEstart)/CLOCKS_PER_SEC << endl;
#endif
		/*部分虚拟机装进已购买的服务器，能够处理的del请求（已经部署的vm）也直接处理*/
		for (int iTerm=0; iTerm<request.numEachDay[iDay]; iTerm++) { // 每一条请求
			string vmID = request.info[iDay][iTerm].vmID;

			if (request.info[iDay][iTerm].type) { // add
				/*First fit 没排序版本*/
				string vmName = request.info[iDay][iTerm].vmName;
				vmIsDouble = VM.isDouble(vmName);
				vmReqCPU = VM.reqCPU(vmName);
				vmReqRAM = VM.reqRAM(vmName);

				if (vmIsDouble) {
					serID = server.firstFitDouble(vmReqCPU, vmReqRAM);
				}
				else {
					tie(serID, serNode) = server.firstFitSingle(vmReqCPU, vmReqRAM);
				}

				if (serID != -1) { // 能直接装进去
					if (vmIsDouble) 
						bugID = VM.deploy(server, iDay, vmID, vmName, serID);
					else 
						bugID = VM.deploy(server, iDay, vmID, vmName, serID, serNode);
					if (bugID) {
						cout << "部署失败" << bugID << endl;
						return;
					}
				}
				else // 装不进去
					curSet.insert(make_pair(vmID, vmName));

			}
			else { // del
				/*检查这个vm是否已经被部署 或者 在addSet中*/
				if (VM.workingVmSet.count(vmID))
					VM.deleteVM(vmID, server);
				else if (addSet.count(vmID))
					delSet.insert(vmID);
				else {
					cout << "删除不存在的虚拟机" << endl;
					return;
				}

			}
		}

#ifdef LOCAL
		TIMEend = clock();
		cout<<(double)(TIMEend-TIMEstart)/CLOCKS_PER_SEC << endl;
#endif
		/*迭代把所有add请求的虚拟机，放进服务器，尽量用最少的cost。每次背包问题最多解N台vm，不然复杂度太高*/
		/*由上至下递归，目前没写记忆*/
		string vmName;
		string serName;
		queue<int> path;
		int serID;
		int action;
		string vmID;
		int cnt; // {每次更新curSet时候更新}
		int bugID;

		while (!addSet.empty()) {
#ifdef LOCAL
			cout << "当天剩余:" << addSet.size() << endl;
#endif
			/*更新curSet*/
			curSet.clear();
			cnt = 0;
			for (auto it=addSet.begin(); it!=addSet.end() && cnt<server.ksSize; it++) {
				curSet.push_back(make_pair(it->first, it->second));
				cnt++;
			}

			// 选择最优背包
			tie(serName, path) = knapSack(server, VM, curSet, iDay);

			/*部署*/
			serID = server.purchase(serName, iDay);
			for (int iVM=0; iVM<(int)curSet.size(); iVM++) {
				action = path.front();
				path.pop();
				vmID = curSet[iVM].first;
				vmName = curSet[iVM].second;
				switch (action) 
				{
					case 0:
						break;
					case 1:
						bugID = VM.deploy(server, iDay, vmID, vmName, serID);
						if (bugID) {
							cout << "服务器部署失败" << bugID << endl;
						}
						bugID = addSet.erase(vmID);
						if (bugID==0) {
							cout << "没有请求的服务器却被部署" << endl;
							return;
						}
						break;
					case 2:
						bugID = VM.deploy(server, iDay, vmID, vmName, serID, true);
						if (bugID) {
							cout << "服务器部署失败" << bugID << endl;
						}
						bugID = addSet.erase(vmID);
						if (bugID==0) {
							cout << "没有请求的服务器却被部署" << endl;
							return;
						}
						break;
					case 3:
						bugID = VM.deploy(server, iDay, vmID, vmName, serID, false);
						if (bugID) {
							cout << "服务器部署失败" << bugID << endl;
						}
						bugID = addSet.erase(vmID);
						if (bugID==0) {
							cout << "没有请求的服务器却被部署" << endl;
							return;
						}
						break;
				}
			}
		}
#ifdef LOCAL
		TIMEend = clock();
		cout<<(double)(TIMEend-TIMEstart)/CLOCKS_PER_SEC << endl;
#endif
		/*处理上一步部署后又del的请求，先处理add请求后处理del请求是次优的，
			但是第二天又可以补上，目前认为影响不大*/
		for (const auto &vmID : delSet) {
			VM.deleteVM(vmID, server);
		}
		delSet.clear();

#ifdef LOCAL
		// 一天结束统计功耗成本
		for (int iServer=0; iServer<(int)server.myServerSet.size(); iServer++) {
			if (server.isOpen(iServer)) {
				costName = server.myServerSet[iServer].serName;
				engCostStas += server.info[costName].energyCost;
			}
		}
#endif
	}
#ifdef LOCAL
	// 计算买服务器总成本
	for (int iServer=0; iServer<(int)server.myServerSet.size(); iServer++) {
		costName = server.myServerSet[iServer].serName;
		hardCostStas += server.info[costName].hardCost;
	}
	cout << "成本: " << engCostStas+hardCostStas << endl;
#endif
}

tuple<string, queue<int>> knapSack(const cServer &server, cVM &VM, \
	vector<pair<string, string>> &curSet, int iDay) 
{
/* Fn: 遍历所有服务器选择性价比最高的那一台
*	- 价格是按照剩余天数加权的
*/
	string serName;
	int totalCPU;
	int totalRAM;
	double unity_ = 0;
	double unity; // 遍历目标，装入资源/加权价格
	int ocpRes; // 装入资源
	// 记录装入的虚拟机，栈顶表示curSet0号的处理结果，编号:0-不装 1-双节点部署 2-nodeA单节点 3-nodeB单节点
	queue<int> vmPath; 
	string bestSer;
	queue<int> bestPath;
	int dayNum = server.buyRecord.size();

	for (const auto &xSer : server.info) {
		serName = xSer.first;
		totalCPU = xSer.second.totalCPU;
		totalRAM = xSer.second.totalRAM;
		tie(ocpRes, vmPath) = dp(curSet.size()-1, totalCPU / 2, totalRAM / 2, totalCPU / 2, totalRAM / 2, \
			curSet, VM);
		unity = (double)ocpRes / (xSer.second.hardCost + xSer.second.energyCost * (dayNum - iDay));
		if (unity > unity_) {
			unity_ = unity;
			bestSer = serName;
			bestPath = vmPath;
		}
	}

	return make_tuple(bestSer, bestPath);
}

tuple<int, queue<int>> dp(int N, int aIdleCPU, int aIdleRAM, int bIdleCPU, int bIdleRAM, \
	vector<pair<string, string>> &curSet, cVM &VM) 
{
/*
* Note: 目前没有记忆，只是单纯递归，规模如果需要很大可能自定义类型哈希表
*/
	queue<int> resPath; // 各个vm的处理结果 ref: knapSack函数的注释
	int res; // 装入资源数，cpu和ram直接相加

	if (aIdleCPU < 0 || aIdleRAM < 0 || bIdleCPU < 0 || bIdleRAM < 0) {
		for (; N!=-1; N--) {
			resPath.push(0);
		}
		return make_tuple(-1, resPath);
	}
	else if (N<0) // 前提是资源还是够用，不然是 不能装进去的应该返回-1
		return make_tuple(0, resPath);

	/*第N台虚拟机资料*/
	string vmName = curSet[N].second;
	int needCPU = VM.info[vmName].needCPU;
	int needRAM = VM.info[vmName].needRAM;
	bool vmIsDouble = VM.info[vmName].nodeStatus;
	int x; // 不装入递归结果
	int y; // 装入的递归结果
	int y1, y2; // 单节点部署下 nodeA和nodeB的结果
	queue<int> path, path0, path1, path2; // path0不装入路径 path装入路径 path1/path2同y1 y2解释

	/*递归*/
	tie(x, path0) = dp(N - 1, aIdleCPU, aIdleRAM, bIdleCPU, bIdleRAM, curSet, VM);
	path0.push(0);

	if (vmIsDouble) {
		tie(y, path) = dp(N - 1, aIdleCPU - needCPU / 2, aIdleRAM - needRAM / 2, bIdleCPU - needCPU / 2, \
			bIdleRAM - needRAM / 2, curSet, VM);
		if (y != -1) { // 当前这个vm是可以装进去的
			y += needCPU + needRAM;
			path.push(1);
		}
		else { // 当前这个vm是装不进去的
			y = -1;
			path.push(0);
		}
		
	}

	else {
		// single node a
		tie(y1, path1) = dp(N - 1, aIdleCPU - needCPU, aIdleRAM - needRAM, bIdleCPU, bIdleRAM, curSet, VM);
		if (y1 != -1){
			y1 += needCPU + needRAM;
			path1.push(2);
		}
		else {
			y1 = -1;
			path1.push(0);
		}
		
		// single node b
		tie(y2, path2) = dp(N - 1, aIdleCPU, aIdleRAM, bIdleCPU - needCPU, bIdleRAM - needRAM, curSet, VM);
		if (y2 != -1) {
			y2 += needCPU + needRAM;
			path2.push(3);
		}
		else {
			y2 = -1;
			path2.push(0);
		}
		
		/* node a和 node b选一个*/
		if (y1 >= y2) {
			path = path1;
			y = y1;
		}
		else {
			path = path2;
			y = y2;
		}
	}

	if (y > x) { // 因为x>=0，所以可以不需单独讨论y==-1
		resPath = path;
		res = y;
	}
	else {
		resPath = path0;
		res = x;
	}

	return make_tuple(res, resPath);
}