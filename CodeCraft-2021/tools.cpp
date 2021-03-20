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
	for (int i = 0; (int)i < server.priceOrder.size(); i++) {

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
	for (int i = 0; (int)i < server.priceOrder.size(); i++) {
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


// 开始编写迁移函数
void migrateVM(cServer &server, cVM &VM, int whichDay, unordered_map<string, int> &dayWorkingVM, int vmTotalNum) {

	int migrateNum = vmTotalNum * 5 / 1000;
	//int migrateNum = (VM.workingVmSet.size() - dayWorkingVM.size()) * 5 / 1000;   // 允许的最大迁移数量
	int serID;   // 迁出的服务器id
	sServerItem myServer;   // 迁入的服务器
	sVmItem requestVM;   // 需要迁移的虚拟机
	string vmID;    // 需要迁出的虚拟机ID
	int count = 0;   // 成功迁移的虚拟机数量统计

	int totalServer = 0;
	for (int i = 0; i < whichDay; i++) {
		totalServer += server.serverNum[i]; 
	}

	for (int i = 0; i < server.vmSourceOrder.size(); i++) {

		serID = server.vmSourceOrder[i].first;   // 迁出的服务器id
		if (serID >= totalServer) {
			continue;    // 表示该服务器是在当天购买的，不能迁移
		}
		int num = server.serverVMSet[serID].size();  // 该服务器里的虚拟机数量
		for (int j = 0; j < num; j++) {

			if (count >= migrateNum) {
				return;
			}

			vmID = server.serverVMSet[serID].begin()->first;    // 需要迁出的虚拟机ID
			if (dayWorkingVM.count(vmID) == 1) {    // 该虚拟机时当天购买的，所以不迁移
				continue;
			}
			requestVM = VM.info[VM.workingVmSet[vmID].vmName];
			myServer = chooseBuyServer(server, requestVM, i);   // 找到合适的服务器
			if (myServer.hardCost == -1) {   // 表示找到了服务器
				count++;    // 迁移数量+1
				if (requestVM.nodeStatus) {   // true表示双节点
					VM.transfer(server, whichDay, vmID, myServer.buyID);
				}
				else {    // false表示单节点
					VM.transfer(server, whichDay, vmID, myServer.buyID, myServer.node);
				}
				server.updatVmSourceOrder(requestVM, serID, false);
				server.updatVmSourceOrder(requestVM, myServer.buyID, true);
			} 
			else {
				break;   // 没找到合适的服务器，说明迁不完了
			}

		}

	}

}


// 迁移使用的函数，需要指定index
sServerItem chooseBuyServer(cServer &server, sVmItem &requestVM, int index) {

	sServerItem myServer;
	myServer.hardCost = 1;   // 可通过hardCost来判断是否找到了服务器

	if (server.myServerSet.size() > 0) {   // 有服务器才开始找

		sMyEachServer tempServer;
		int restCPU;
		int restRAM;
		int minValue = INT_MAX;
		int tempValue;
		int serID;      // 要迁入的id
		double first = (double)server.vmSourceOrder[index].second;
		double second;

		for (unsigned i = server.vmSourceOrder.size() - 1; i > index; i--) {   // 从后面往前找

			second = (double)server.vmSourceOrder[i].second;
			if (second / first < 2.0) {
				break;
			}

			serID = server.vmSourceOrder[i].first;      // 取出下一个的id
			tempServer = server.myServerSet[serID];  // 既然要根据虚拟机来，排序就没有用了，遍历所有服务器

			if (!requestVM.nodeStatus) {    // 单节点
				if (tempServer.aIdleCPU >= requestVM.needCPU && tempServer.aIdleRAM >= requestVM.needRAM) {    // a节点
					restCPU = tempServer.aIdleCPU - requestVM.needCPU;
					restRAM = tempServer.aIdleRAM - requestVM.needRAM;
					tempValue = restCPU + restRAM + abs(restCPU - 0.3 * restRAM) * 1;
					if (tempValue < minValue) {
						minValue = tempValue;
						myServer.energyCost = -1;
						myServer.hardCost = -1;
						myServer.buyID = serID;   // 记录该服务器
						myServer.node = true;   // 返回true表示a 节点
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
						myServer.buyID = serID;   // 记录服务器
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
						myServer.buyID = serID;   // 记录服务器
					}
				}
			}
		}
	}

	return myServer;  // 运行到这表示没找到,hardCost为1

}


// 对每天的请求按照资源的大小进行排序
void requestOrder(cVM &VM, cRequests &request) {

	int maxNum = 50;   // 排序的最多个数
	vector<pair<int, int>> order;   // int: 实际位置, int: 资源数
	sVmItem requestItem;
	int tempValue;
	int count = 0;

	for (int dayNum = 0; dayNum < request.dayNum; dayNum++) {
		for (int i = 0; i < request.numEachDay[dayNum]; i++) {
			if (request.info[dayNum][i].type) {    // add
				requestItem = VM.info[request.info[dayNum][i].vmName];  // 取出该虚拟机
				tempValue = requestItem.needCPU + requestItem.needRAM;
				order.push_back(make_pair(i, tempValue));
				count++;
				if (count >= 50 || i == request.numEachDay[dayNum] - 1) {
					sort(order.begin(), order.end(), mycomp);
					updateRequestOrder(request, order, dayNum, i + 1 - count);
					order.clear();
					count = 0;
				}
			}
			else {   // 删除的顺序就不动了
				if (order.size() == 0) {   // 没有就不需要排序了
					count = 0;
					continue;
				}
				sort(order.begin(), order.end(), mycomp);
				updateRequestOrder(request, order, dayNum, i + 1 - count);
				order.clear();
				count = 0;
			}
		}
	}

}

bool mycomp(pair<int, int> i, pair<int, int> j) {
	return i.second < j.second;
}

void updateRequestOrder(cRequests &request, vector<pair<int, int>> &order, int whichDay, int begin) {

	vector<sRequestItem> tempRequest;
	int index;   // 实际的位置
	for (int i = 0; i < order.size(); i++) {
		index = order[i].first;
		tempRequest.push_back(request.info[whichDay][index]);
	}
	for (int i = begin; i < begin + order.size(); i++) {
		request.info[whichDay][i] = tempRequest[i - begin];
	}

}