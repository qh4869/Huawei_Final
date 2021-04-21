﻿#include "ssp.h"
#include "MigrationPlus.h"

int dday;
bool ompFlag = true;

void sspEachDay(int iDay, cSSP_Mig_Server &server, cSSP_Mig_VM &VM, cSSP_Mig_Request &request, int laterDay) {
/* Fn: ssp复赛版本
*
* Update 03.31
*/
	dday = iDay;

	/*一些变量用于迁移*/
	int vmNumStart = VM.workingVmSet.size(); // 购买部署前 工作虚拟机的个数
	unordered_map<int, sMyEachServer> delSerSet; // 有删除操作的SV集合，以及其中扣除del操作的容量
	unordered_set<string> dayWorkingVM; // 当天新加入的VM集合(就算del也没从中去掉)
	
	int cntMig = 0;

	int migrateNum;
	if (iDay == laterDay) {
		migrateNum = vmNumStart; // 大规模迁移
	}
	else {
		migrateNum = vmNumStart * 30 / 1000;
	}

	unordered_map<string ,int> dayWorkingMap;

	dailyPurchaseDeploy(server, VM, request, iDay, delSerSet, dayWorkingVM);
	

	for (auto &x : dayWorkingVM) {
		dayWorkingMap.insert({x, 1});
	}

	if (iDay == laterDay) { // 大量迁移
		for (int k = 0; k < 8; k++) {
			massMigrate(server, VM, iDay, dayWorkingMap, delSerSet, cntMig, migrateNum, server.args);
		}
	}
	else
		massMigrate(server, VM, iDay, dayWorkingMap, delSerSet, cntMig, migrateNum, server.args);

	/*更新占空比*/
	server.updateResourceRatio();
	server.updateResourceRatioIncludeALL();
}

tuple<string, queue<int>> knapSack(const cSSP_Mig_Server &server, cSSP_Mig_VM &VM, \
	vector<pair<string, string>> &curSet, int iDay)
{
/* Fn: 遍历所有服务器选择性价比最高的那一台
*	- 价格是按照剩余天数加权的
*
* Out:
*	- string: vmName
*	- queue<int> curSet中每台vm的部署策略 0不部署 1双节点部署 2nodeA单节点 3nodeB单节点
*		队列头部对应curSet的头部
*/

	int dayNum = server.buyRecord.size(); // 总天数

	/*2个线程*/
	double utility_[2] = { 0 };
	string bestSer[2];
	queue<int> bestPath[2];

#pragma omp parallel for num_threads(2)
	for (int iSer = 0; iSer<server.serverTypeNum; iSer++) {

		queue<int> vmPath;
		int ocpRes; // 装入的资源
		double utility;

		string serName = server.infoV[iSer].first;
		int totalCPU = server.infoV[iSer].second.totalCPU;
		int totalRAM = server.infoV[iSer].second.totalRAM;
		int hardprice = server.infoV[iSer].second.hardCost;
		int engprice = server.infoV[iSer].second.energyCost;

		tie(ocpRes, vmPath) = dp(curSet.size() - 1, totalCPU / 2, totalRAM / 2, totalCPU / 2, totalRAM / 2, \
			curSet, VM);

		utility = (double)ocpRes / (hardprice + engprice * (dayNum - iDay));
		if (utility > utility_[omp_get_thread_num()]) {
			utility_[omp_get_thread_num()] = utility;
			bestSer[omp_get_thread_num()] = serName;
			bestPath[omp_get_thread_num()] = vmPath;
		}

	}

	/*两个线程结果汇总*/
	if (utility_[0] >= utility_[1])
		return make_tuple(bestSer[0], bestPath[0]);
	else
		return make_tuple(bestSer[1], bestPath[1]);
}

tuple<int, queue<int>> dp(int N, int aIdleCPU, int aIdleRAM, int bIdleCPU, int bIdleRAM, \
	vector<pair<string, string>> &curSet, cSSP_Mig_VM &VM)
{
/*
* Note: 目前没有记忆，只是单纯递归，规模如果需要很大可能自定义类型哈希表
*/
	queue<int> resPath; // 各个vm的处理结果 ref: knapSack函数的注释
	int res; // 装入资源数，cpu和ram直接相加

	if (aIdleCPU < 0 || aIdleRAM < 0 || bIdleCPU < 0 || bIdleRAM < 0) {
		for (; N != -1; N--) {
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
		if (y1 != -1) {
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

void packAndDeploy(cSSP_Mig_Server &server, cSSP_Mig_VM &VM, vector<pair<string, string>> &curSet, int iDay) {
	string serName;
	queue<int> path;
	vector<bool> isDeployed(curSet.size(), false);
	int serID;
	int action;
	string vmID; // 临时变量，和上面的冲突了
	string vmName;

	// 选择最优背包
	tie(serName, path) = knapSack(server, VM, curSet, iDay);

	// 买服务器
	serID = server.purchase(serName, iDay);

	/*部署*/
	for (int iVM = 0; iVM<(int)curSet.size(); iVM++) {
		action = path.front();
		path.pop();
		vmID = curSet[iVM].first;
		vmName = curSet[iVM].second;
		switch (action)
		{
		case 0:
			break;
		case 1:
			VM.deploy(server, iDay, vmID, vmName, serID);
			isDeployed[iVM] = true;
			break;
		case 2:
			VM.deploy(server, iDay, vmID, vmName, serID, true);
			isDeployed[iVM] = true;
			break;
		case 3:
			VM.deploy(server, iDay, vmID, vmName, serID, false);
			isDeployed[iVM] = true;
			break;
		}
	}

	/*已经部署的服务器从curSet中删除*/
	auto itIndicator = isDeployed.begin();
	auto it = curSet.begin();
	while (it != curSet.end()) {
		if (*itIndicator == true) {
			it = curSet.erase(it);
		}
		else
			it++;
		itIndicator++;
	}
}


cyt::sServerItem bestFit(cSSP_Mig_Server &server, sVmItem &requestVM) {
	/* Fn: cyt写的，best fit，源程序在cyt分支的tools.cpp中
	*/

	cyt::sServerItem myServer;
	myServer.hardCost = 1;   // 可通过hardCost来判断是否找到了服务器

	if (server.myServerSet.size() > 0) {   // 有服务器才开始找

		int minValue[2] = { INT_MAX, INT_MAX };
		cyt::sServerItem ompServer[2];
		ompServer[0].hardCost = 1;
		ompServer[1].hardCost = 1;

#pragma omp parallel for num_threads(2)
		for (unsigned i = 0; i < server.myServerSet.size(); i++) {

			sMyEachServer tempServer = server.myServerSet[i];   // 既然要根据虚拟机来，排序就没有用了，遍历所有服务器

			if (!requestVM.nodeStatus) {    // 单节点
				if (tempServer.aIdleCPU >= requestVM.needCPU && tempServer.aIdleRAM >= requestVM.needRAM) {    // a节点
					int restCPU = tempServer.aIdleCPU - requestVM.needCPU;
					int restRAM = tempServer.aIdleRAM - requestVM.needRAM;
					int tempValue = restCPU + restRAM + abs(restCPU - server.gBeta * restRAM) * server.gGamma;
					if (tempValue < minValue[omp_get_thread_num()]) {
						minValue[omp_get_thread_num()] = tempValue;
						ompServer[omp_get_thread_num()].energyCost = -1;
						ompServer[omp_get_thread_num()].hardCost = -1;
						ompServer[omp_get_thread_num()].buyID = i;   // 记录该服务器
						ompServer[omp_get_thread_num()].node = true;   // 返回false表示b 节点
					}
				}
				// 两个节点都要查看，看看放哪个节点更合适
				if (tempServer.bIdleCPU >= requestVM.needCPU && tempServer.bIdleRAM >= requestVM.needRAM) {  // b 节点
					int restCPU = tempServer.bIdleCPU - requestVM.needCPU;
					int restRAM = tempServer.bIdleRAM - requestVM.needRAM;
					int tempValue = restCPU + restRAM + abs(restCPU - server.gBeta * restRAM) * server.gGamma;
					if (tempValue < minValue[omp_get_thread_num()]) {
						minValue[omp_get_thread_num()] = tempValue;
						ompServer[omp_get_thread_num()].energyCost = -1;
						ompServer[omp_get_thread_num()].hardCost = -1;
						ompServer[omp_get_thread_num()].buyID = i;   // 记录服务器
						ompServer[omp_get_thread_num()].node = false;   // 返回false表示b 节点
					}
				}
			}
			else {     // 双节点
				if (tempServer.aIdleCPU >= requestVM.needCPU / 2 && tempServer.aIdleRAM >= requestVM.needRAM / 2
					&& tempServer.bIdleCPU >= requestVM.needCPU / 2 && tempServer.bIdleRAM >= requestVM.needRAM / 2) {
					int restCPU = tempServer.aIdleCPU + tempServer.bIdleCPU - requestVM.needCPU;
					int restRAM = tempServer.aIdleRAM + tempServer.bIdleRAM - requestVM.needRAM;
					int tempValue = restCPU + restRAM + abs(restCPU - server.gBeta * restRAM) * server.gGamma;
					if (tempValue < minValue[omp_get_thread_num()]) {
						minValue[omp_get_thread_num()] = tempValue;
						ompServer[omp_get_thread_num()].energyCost = -1;
						ompServer[omp_get_thread_num()].hardCost = -1;
						ompServer[omp_get_thread_num()].buyID = i;   // 记录服务器
					}
				}
			}
		}

		/*多线程合并*/
		if (minValue[0] <= minValue[1])
			myServer = ompServer[0];
		else
			myServer = ompServer[1];
	}

	return myServer;  // 运行到这表示没找到,hardCost为1

}

int srchInMyServerDouble(cSSP_Mig_Server &server, sVmItem &requestVM) {
	/* Fn: 从已购买的服务器中找一台，来部署这台VM
	*
	* Out:
	*	- 返回服务器id（id映射前），若找不到合适的SV，返回-1
	*/
	cyt::sServerItem myServer = bestFit(server, requestVM);

	if (myServer.hardCost == -1) {    // 找到了服务器
		return myServer.buyID;
	}

	return -1;
}

std::tuple<int, bool> srchInMyServerSingle(cSSP_Mig_Server &server, sVmItem &requestVM) {
	/* Fn: 功能同上，单节点部署类型，不是单节点部署抛异常
	*
	*/
	cyt::sServerItem myServer = bestFit(server, requestVM);

	if (myServer.hardCost == -1) {    // 找到了服务器
		return make_tuple(myServer.buyID, myServer.node);
	}

	return make_tuple(-1, false);
}

void dailyPurchaseDeploy(cSSP_Mig_Server &server, cSSP_Mig_VM &VM, cRequests &request, int iDay,
	unordered_map<int, sMyEachServer> &delSerSet, unordered_set<string> &dayWorkingVM) {
	/* Fn: bestFit + knapSack
	* Out: (传参返回)
	*	- 当天有过删除操作的服务器列表，<serID, 不考虑del操作的剩余资源>
	*	- 当天有过add请求的vm集合
	*
	* Note:
	*	- delSerSet的剩余资源有可能是负数的，因为扣掉了del腾出的资源
	*/
	/* 背包算法要处理的vm add请求集合，最多server.ksSize个元素 */
	vector<pair<string, string>> curSet;
	/*还在curSet中的请求如果又来了del请求，那么就先放到这个里*/
	vector<pair<string, string>> delSet;
	// 当天删除的虚拟机<vmID, sEachWorkingVM> 用来计算输出 {每次del虚拟机之前更新}
	unordered_map<string, sEachWorkingVM> dayDeleteVM;

	/*部分虚拟机装进已购买的服务器，能够处理的del请求（已经部署的vm）也直接处理*/
	for (int iTerm = 0; iTerm<request.numEachDay[iDay]; iTerm++) { // 每一条请求
		string vmID = request.info[iDay][iTerm].vmID;

		/*firstFit*/
		if (request.info[iDay][iTerm].type) { // add
			dayWorkingVM.insert(vmID);

			/*最新版的选择策略*/
			string vmName = request.info[iDay][iTerm].vmName;
			sVmItem requestVM = VM.info[vmName];
			bool vmIsDouble = VM.isDouble(vmName);
			int serID;
			bool serNode;

			if (vmIsDouble) {
				serID = srchInMyServerDouble(server, requestVM);
			}
			else {
				tie(serID, serNode) = srchInMyServerSingle(server, requestVM);
			}

			if (serID != -1) { // 能直接装进去
				if (vmIsDouble)
					VM.deploy(server, iDay, vmID, vmName, serID);
				else
					VM.deploy(server, iDay, vmID, vmName, serID, serNode);
			}
			else // 装不进去
				curSet.push_back(make_pair(vmID, vmName));
		}
		else { // del
			   /*检查这个vm是否已经被部署 或者 在curSet中*/
			if (VM.workingVmSet.count(vmID)) {
				dayDeleteVM.insert({ vmID, VM.workingVmSet[vmID] });
				VM.deleteVM(vmID, server);
			}
			else {
				bool flag = false;
				for (const auto &x : curSet) {
					if (x.first == vmID) { // 在curSet中
						delSet.push_back(x);
						flag = true;
						break;
					}
				}
				if (flag == false) {
					cout << "不能删除不存在的虚拟机2" << endl;
				}
			}
		}

		/*knapSack*/
		if ((int)curSet.size() == server.ksSize) { // 可以执行背包算法了
			packAndDeploy(server, VM, curSet, iDay);
		}

		/*delSet遗留问题，训练集数据都没出现这个情况*/
		for (auto it = delSet.begin(); it != delSet.end(); ) {
			string vmID = it->first;
			if (VM.workingVmSet.count(vmID)) { // 不满足的还留在delSet里
				dayDeleteVM.insert({ vmID, VM.workingVmSet[vmID] });
				VM.deleteVM(vmID, server);
				it = delSet.erase(it);
			}
			else
				it++;
		}
	}

	/*curSet中还遗留部分vm*/
	while (curSet.size()) {
		packAndDeploy(server, VM, curSet, iDay);
		for (auto it = delSet.begin(); it != delSet.end(); ) {
			string vmID = it->first;
			if (VM.workingVmSet.count(vmID)) { // 不满足的还留在delSet里
				dayDeleteVM.insert({ vmID, VM.workingVmSet[vmID] });
				VM.deleteVM(vmID, server);
				it = delSet.erase(it);
			}
			else
				it++;
		}
	}
	if (delSet.size()) {
		cout << "某（些）需要删除的服务器没有被部署" << endl;
		return;
	}

	delSerSet = recoverDelSerSet(server, VM, dayDeleteVM);
}

// 有删除操作的服务器需要把被删除的虚拟机容量加回去
unordered_map<int, sMyEachServer> recoverDelSerSet(cSSP_Mig_Server &server, cSSP_Mig_VM &VM,
	unordered_map<string, sEachWorkingVM> &dayDeleteVM) {

	unordered_map<int, sMyEachServer> delSerSet;
	sMyEachServer myServer;
	sEachWorkingVM workVM;
	sVmItem requestVM;
	int serID;   // 服务器ID

	for (auto ite = dayDeleteVM.begin(); ite != dayDeleteVM.end(); ite++) {
		workVM = ite->second;
		serID = workVM.serverID;
		requestVM = VM.info[workVM.vmName];

		if (delSerSet.count(serID) == 1) {    // 该服务器之前已经删过虚拟机了
			myServer = delSerSet[serID];
		}
		else {   // 该服务器第一次删除虚拟机
			myServer = server.myServerSet[serID];
		}

		if (requestVM.nodeStatus) {   // true : double node
			myServer.aIdleCPU -= requestVM.needCPU / 2;
			myServer.bIdleCPU -= requestVM.needCPU / 2;
			myServer.aIdleRAM -= requestVM.needRAM / 2;
			myServer.bIdleRAM -= requestVM.needRAM / 2;
		}
		else {  // false : single node
			if (workVM.node) {   // true : a node
				myServer.aIdleCPU -= requestVM.needCPU;
				myServer.aIdleRAM -= requestVM.needRAM;
			}
			else {    // false : b node
				myServer.bIdleCPU -= requestVM.needCPU;
				myServer.bIdleRAM -= requestVM.needRAM;
			}
		}
		if (delSerSet.count(serID) == 1) {   // 存在的话就直接修改值
			delSerSet[serID] = myServer;
		}
		else {    // 不存在的话就添加新的键
			delSerSet.insert({ serID, myServer });
		}
	}

	return delSerSet;
}

// 更新有删除操作的服务器的CPU和RAM
void updateDelSerSet(unordered_map<int, sMyEachServer> &delSerSet,
	sVmItem &requestVM, bool node, int serID, bool flag) {

	if (flag) {   // add
		if (requestVM.nodeStatus) {   // true : double
			delSerSet[serID].aIdleCPU -= requestVM.needCPU / 2;
			delSerSet[serID].bIdleCPU -= requestVM.needCPU / 2;
			delSerSet[serID].aIdleRAM -= requestVM.needRAM / 2;
			delSerSet[serID].bIdleRAM -= requestVM.needRAM / 2;
		}
		else {   // false : single
			if (node) {  // a node
				delSerSet[serID].aIdleCPU -= requestVM.needCPU;
				delSerSet[serID].aIdleRAM -= requestVM.needRAM;
			}
			else {   // b node
				delSerSet[serID].bIdleCPU -= requestVM.needCPU;
				delSerSet[serID].bIdleRAM -= requestVM.needRAM;
			}
		}
	}
	else {   // delete
		if (requestVM.nodeStatus) {   // true : double
			delSerSet[serID].aIdleCPU += requestVM.needCPU / 2;
			delSerSet[serID].bIdleCPU += requestVM.needCPU / 2;
			delSerSet[serID].aIdleRAM += requestVM.needRAM / 2;
			delSerSet[serID].bIdleRAM += requestVM.needRAM / 2;
		}
		else {   // false : single
			if (node) {  // a node
				delSerSet[serID].aIdleCPU += requestVM.needCPU;
				delSerSet[serID].aIdleRAM += requestVM.needRAM;
			}
			else {   // b node
				delSerSet[serID].bIdleCPU += requestVM.needCPU;
				delSerSet[serID].bIdleRAM += requestVM.needRAM;
			}
		}
	}

}