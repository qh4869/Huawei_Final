#include "tools.h"

// 判断虚拟机集合是否能加入指定的服务器，可以返回true，否则为false
bool isFitServer(sMyEachServer &myServer, vector<sVmItem> &vmSet) {

	// 先处理双节点
	for (unsigned i = 0; i < vmSet.size(); i++) {
		if (vmSet[i].nodeStatus) {  // true表示双节点
			myServer.aIdleCPU -= vmSet[i].needCPU / 2;
			myServer.bIdleCPU -= vmSet[i].needCPU / 2;
			myServer.aIdleRAM -= vmSet[i].needRAM / 2;
			myServer.bIdleRAM -= vmSet[i].needRAM / 2;
			if (myServer.aIdleCPU < 0 || myServer.bIdleCPU < 0
				|| myServer.aIdleRAM < 0 || myServer.bIdleRAM < 0) {
				return false;
			}
		}
	}

	// 继续处理单节点
	for (unsigned i = 0; i < vmSet.size(); i++) {
		if (!vmSet[i].nodeStatus) {   // false表示单节点
			if (myServer.aIdleCPU - vmSet[i].needCPU >= 0 && myServer.aIdleRAM - vmSet[i].needRAM >= 0) {
				myServer.aIdleCPU -= vmSet[i].needCPU;
				myServer.aIdleRAM -= vmSet[i].needRAM;
			}
			else if (myServer.bIdleCPU - vmSet[i].needCPU >= 0 && myServer.bIdleRAM - vmSet[i].needRAM >= 0) {
				myServer.bIdleCPU -= vmSet[i].needCPU;
				myServer.bIdleRAM -= vmSet[i].needRAM;
			}
			else {
				return false;
			}
		}
	}

	return true;

}

// 判断虚拟机集合是否能加入指定的服务器，可以返回true,否则返回false
bool isFitServer(sServerItem &serverItem, vector<sVmItem> &vmSet) {

	int aCpu = serverItem.totalCPU / 2;
	int bCpu = serverItem.totalCPU / 2;
	int aRam = serverItem.totalRAM / 2;
	int bRam = serverItem.totalRAM / 2;

	// 先处理双节点
	for (unsigned i = 0; i < vmSet.size(); i++) {
		if (vmSet[i].nodeStatus) { // true表示双节点
			aCpu -= vmSet[i].needCPU / 2;
			bCpu -= vmSet[i].needCPU / 2;
			aRam -= vmSet[i].needRAM / 2;
			bRam -= vmSet[i].needRAM / 2;

			if (aCpu < 0 || bCpu < 0 || aRam < 0 || bRam < 0) {
				return false;
			}
		}
	}

	// 继续处理单节点
	for (unsigned i = 0; i < vmSet.size(); i++) {
		if (!vmSet[i].nodeStatus) {   // 单节点处理
			if (aCpu - vmSet[i].needCPU >= 0 && aRam - vmSet[i].needRAM >= 0) {
				aCpu -= vmSet[i].needCPU;
				aRam -= vmSet[i].needRAM;
			}
			else if (bCpu - vmSet[i].needCPU >= 0 && bRam - vmSet[i].needRAM >= 0) {
				bCpu -= vmSet[i].needCPU;
				bRam -= vmSet[i].needRAM;
			}
			else {
				return false;
			}
		}
	}

	return true;
}


// 加入单个虚拟机时，找到能放进该虚拟机的最便宜的服务器
sServerItem chooseServer(cServer &server, sVmItem &requestVM) {

	int minCost = INT_MAX;
	sServerItem myServer;

	myServer = chooseBuyServer(server, requestVM);   // 从已经购买的服务器中挑选合适的
	if (myServer.hardCost == -1) {    // 找到了服务器
		return myServer;
	}

	// 已购买服务器中没有满足条件的，需要买新的服务器
	for (int i = 0; i < server.priceOrder.size(); i++) {

		myServer = server.info[server.priceOrder[i].first];   // 根据名称取出服务器
		if (!requestVM.nodeStatus && requestVM.needCPU <= myServer.totalCPU / 2   // 单节点
			&& requestVM.needRAM <= myServer.totalRAM / 2) {
			return myServer;
		}
		else if (requestVM.nodeStatus && requestVM.needCPU <= myServer.totalCPU  // 双节点
			&& requestVM.needRAM <= myServer.totalRAM) {
			return myServer;
		}

	}

	return myServer;
}


// 有多个虚拟机，查看是否有符合条件的服务器，有的话，查找价格最便宜的那个
sServerItem chooseServer(cServer &server, unordered_map<string, sVmItem> &workVm,
	vector<pair<string, int>> &restID, int begin) {

	vector<sVmItem> vmSet;     // 用于记录需要加入的虚拟机
	string id;     // 虚拟机ID
	for (unsigned i = begin; i < restID.size() - 1; i++) {  // 注意这里的restID.size() - 1
		id = restID[i].first;    // restID: id->index
		vmSet.push_back(workVm[id]);  // workVM: id->sVmItem
	}

	int minCost = INT_MAX;
	sServerItem myServer;
	// 从最便宜的开始找
	for (int i = 0; i < server.priceOrder.size(); i++) {
		myServer = server.info[server.priceOrder[i].first];    // 根据名称取出服务器
		if (isFitServer(myServer, vmSet)) {    // 如果找到了的话直接返回，此时价格已经是最低的了
			return myServer;
		}
	}
	myServer.serName = "";   // 表示找不到
	return myServer;
}

// 从已购买的服务器中挑选最合适的服务器
sServerItem chooseBuyServer(cServer &server, sVmItem &requestVM) {

	sServerItem myServer;
	myServer.hardCost = 1;   // 可通过hardCost来判断是否找到了服务器

	if (server.myServerSet.size() > 0) {   // 有服务器才开始找

		sMyEachServer tempServer;
		int restCPU;
		int restRAM;
		int minValue = INT_MAX;
		int tempValue;

		for (unsigned i = 0; i < server.myServerSet.size(); i++) {

			tempServer = server.myServerSet[i];   // 既然要根据虚拟机来，排序就没有用了，遍历所有服务器

			if (!requestVM.nodeStatus) {    // 单节点
				if (tempServer.aIdleCPU >= requestVM.needCPU && tempServer.aIdleRAM >= requestVM.needRAM) {    // a节点
					restCPU = tempServer.aIdleCPU - requestVM.needCPU;
					restRAM = tempServer.aIdleRAM - requestVM.needRAM;
					tempValue = restCPU + restRAM + abs(restCPU - 0.3 * restRAM) * 1;
					if (tempValue < minValue) {
						minValue = tempValue;
						myServer.energyCost = -1;
						myServer.hardCost = -1;
						myServer.buyID = i;   // 记录该服务器
						myServer.node = true;   // 返回false表示b 节点
					}
				}
				// 两个节点都要查看，看看放哪个节点更合适
				if (tempServer.bIdleCPU >= requestVM.needCPU && tempServer.bIdleRAM >= requestVM.needRAM) {  // b 节点
					restCPU = tempServer.bIdleCPU - requestVM.needCPU;
					restRAM = tempServer.bIdleRAM - requestVM.needRAM;
					tempValue = restCPU + restRAM + abs(restCPU - 0.3 * restRAM) * 1;
					if (tempValue < minValue) {
						minValue = tempValue;
						myServer.energyCost = -1;
						myServer.hardCost = -1;
						myServer.buyID = i;   // 记录服务器
						myServer.node = false;   // 返回false表示b 节点
					}
				}
			}
			else {     // 双节点
				if (tempServer.aIdleCPU >= requestVM.needCPU / 2 && tempServer.aIdleRAM >= requestVM.needRAM / 2
					&& tempServer.bIdleCPU >= requestVM.needCPU / 2 && tempServer.bIdleRAM >= requestVM.needRAM / 2) {
					restCPU = tempServer.aIdleCPU + tempServer.bIdleCPU - requestVM.needCPU;
					restRAM = tempServer.aIdleRAM + tempServer.bIdleRAM - requestVM.needRAM;
					tempValue = restCPU + restRAM + abs(restCPU - 0.3 * restRAM) * 1;
					if (tempValue < minValue) {
						minValue = tempValue;
						myServer.energyCost = -1;
						myServer.hardCost = -1;
						myServer.buyID = i;   // 记录服务器
					}
				}
			}
		}
	}

	return myServer;  // 运行到这表示没找到,hardCost为1

}


// 从图中选择最短路径之后部署虚拟机
void deployVM(cServer &server, cVM &VM, const cRequests &request, int index, int begin, int end, vector<bool> &hasDeploy,
	int whichDay, int baseNum) {

	// 先从双节点开始部署，然后再是单节点
	for (int iTerm = begin; iTerm < end; iTerm++) {
		if (hasDeploy[iTerm - baseNum]) {  // 如果已经加入了就不用再部署了
			continue;
		}
		sRequestItem requestTerm = request.info[whichDay][iTerm];
		sVmItem requestVm = VM.info[requestTerm.vmName];
		if (requestVm.nodeStatus) {   // true表示双节点
			VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index);   // index表示serID
		}
	}

	// 部署单节点
	for (int iTerm = begin; iTerm < end; iTerm++) {
		if (hasDeploy[iTerm - baseNum]) {  // 如果已经加入了就不用再部署了
			continue;
		}
		sRequestItem requestTerm = request.info[whichDay][iTerm];
		sVmItem requestVm = VM.info[requestTerm.vmName];
		if (!requestVm.nodeStatus) {   // 表示单节点
			int flag = VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index, true);
			if (flag == 2) {  // a节点放不进去
				VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index, false);
			}
		}
	}
}

