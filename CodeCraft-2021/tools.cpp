#include "tools.h"

// 加入单个虚拟机时，找到能放进该虚拟机的最便宜的服务器
sServerItem chooseServer(cServer &server, sVmItem &requestVM) {

	int minCost = INT_MAX;
	sServerItem myServer;

	// myServerSet存有服务器时，处理已购买的服务器
	if (server.myServerSet.size() > 0) {

		sMyEachServer tempServer;
		int serID;
		for (unsigned i = 0; i < server.myServerSet.size(); i++) {

			serID = server.resourceOrder[i].first;
			tempServer = server.myServerSet[serID];
			/// 下面这个代码是不排序的////////////////////////////////////
			//serID = i;   // server ID 就是存储的顺序
			//tempServer = server.myServerSet[i];    // 从现有的服务器中按顺序查看是否符合条件
			/////////////////////////////////////
			
			if (!requestVM.nodeStatus) {  // 单节点

				if (tempServer.aIdleCPU >= requestVM.needCPU && tempServer.aIdleRAM >= requestVM.needRAM) {   // a 节点
					myServer.energyCost = -1;
					myServer.hardCost = -1;
					myServer.buyID = serID;
					myServer.node = true;
					return myServer;
				}
				else if (tempServer.bIdleCPU >= requestVM.needCPU && tempServer.bIdleRAM >= requestVM.needRAM) { // b 节点
					myServer.energyCost = -1;
					myServer.hardCost = -1;
					myServer.buyID = serID;
					myServer.node = false;
					return myServer;
				}

			}
			else {   // 双节点

				if (tempServer.aIdleCPU >= requestVM.needCPU / 2 && tempServer.aIdleRAM >= requestVM.needRAM / 2
					&& tempServer.bIdleCPU >= requestVM.needCPU / 2 && tempServer.bIdleRAM >= requestVM.needRAM / 2) {
					myServer.energyCost = -1;
					myServer.hardCost = -1;
					myServer.buyID = serID;
					return myServer;
				}

			}

		}

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
sServerItem chooseServer(cServer &server, unordered_map<string, sVmItem> &workVm, vector<pair<string, int>> &restID, int begin) {

	vector<sVmItem> vmSet;     // 用于记录需要加入的虚拟机
	string id;     // 虚拟机ID
	for (unsigned i = begin; i < restID.size() - 1; i++) {
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

// 放进当天购买服务器的VM部署，放进已购买服务器的VM部署在之前已经完成
void deployVM(cServer &server, cVM &VM, const cRequests &request, int index, int begin, int end, vector<bool> &hasDeploy,
	int whichDay, int baseNum) {

	// 先从双节点开始部署，然后再是单节点
	for (int iTerm = begin; iTerm < end; iTerm++) {

		if (hasDeploy[iTerm - baseNum]) {  // 如果已经加入了
			continue;
		}

		sRequestItem requestTerm = request.info[whichDay][iTerm];
		sVmItem requestVm = VM.info[requestTerm.vmName];
		if (requestVm.nodeStatus) {   // true表示双节点
			//VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index);
			VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index, iTerm);
		}

	}

	// 部署单节点
	for (int iTerm = begin; iTerm < end; iTerm++) {

		if (hasDeploy[iTerm - baseNum]) {  // 如果已经加入了
			continue;
		}

		sRequestItem requestTerm = request.info[whichDay][iTerm];
		sVmItem requestVm = VM.info[requestTerm.vmName];
		if (!requestVm.nodeStatus) {   // 表示单节点
			//int flag = VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index, true);
			int flag = VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index, true, iTerm);
			if (flag == 2) {
				//VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index, false);
				VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index, false, iTerm);
			}
		}

	}

}


// 每天结束之后为新购买的服务器生成ID映射表
void deployServerID(cServer &server, int whichDay) {

	int begin = 0;
	int serverNum = server.dayServerNum[whichDay];
	int count = 0;   // 用于记录当天服务器的增值
	for (int i = whichDay - 1; i >= 0; i--) {
		begin += server.dayServerNum[i];   // 新一天的编号的最小值肯定是前几天购买服务器的总数
	}

	unordered_map<string, int> record = server.buyRecord[whichDay];
	string serName;
	for (auto ite = record.begin(); ite != record.end(); ite++) {

		serName = ite->first;
		for (int i = begin; i < begin + serverNum; i++) {
			if (serName == server.myServerSet[i].serName) {    // 如果找到相同的服务器
				server.virToReal.insert({ i, count + begin });  // 虚拟ID->真实ID
				server.realToVir.insert({ count + begin, i });  // 真实ID->虚拟ID
				count++;
			}
		}

	}

}


// 部署时候的请求是无序的，重新生成有序的请求记录
void deployRecord(cServer &server, cVM &VM, const cRequests &request, int whichDay) {

	vector<sDeployItem> dayRecord = VM.deployRecord[whichDay];  // 当天的记录
	vector<sDeployItem> realRecord(request.numEachDay[whichDay]);   // 实际的记录
	sDeployItem defaultItem;
	defaultItem.serID = -1;
	for (int i = 0; i < request.numEachDay[whichDay]; i++) {
		realRecord[i] = defaultItem;   // 删除操作时serID为-1
	}
	sDeployItem temp;
	int serID;
	int realIndex;
	for (int i = 0; i < dayRecord.size(); i++) {
		temp = dayRecord[i];
		realIndex = temp.indexTerm;
		realRecord[realIndex].node = temp.node;      // node
		realRecord[realIndex].isSingle = temp.isSingle;   // isSingle
		realRecord[realIndex].serID = server.virToReal[temp.serID];    // 记录真实值
	}
	VM.realDeployRecord.push_back(realRecord);


}
