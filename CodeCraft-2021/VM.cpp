#include "VM.h"

int cVM::deploy(cServer &server, int iDay, string VMid, string vmName, int serID, bool node) {
	/* Fn: 单节点部署情况
	* 	- 因为要求输出的顺序按照输入虚拟机请求的顺序，所以这个函数的调用必须按照add请求的顺序
	*
	* In:
	*	- server: server对象
	*	- iDay: 天数
	*	- VMid: 虚拟机ID
	*	- vmName: 虚拟机型号
	*	- serID: 被部署服务器ID
	*	- node: 单节点部署的节点 true表示 a节点
	*
	* Out:
	*	- 0表示正常运行，否则存在错误
	*/
	// 判断单双节点虚拟机部署时候 有没有使用错误
	if (info[vmName].nodeStatus) {
		cout << "双节点虚拟机不能指定节点类型，分配失败" << endl;
		return 1;
	}

	if (serID == 126) {
		int a = 1;
	}

	// 判断资源是否够分
	if (node == true) { // a 节点部署
		if (server.myServerSet[serID].aIdleCPU < info[vmName].needCPU \
			|| server.myServerSet[serID].aIdleRAM < info[vmName].needRAM) {
			//cout << "a node error" << endl;
			return 2;
		}
		else {
			server.myServerSet[serID].aIdleCPU -= info[vmName].needCPU;
			server.myServerSet[serID].aIdleRAM -= info[vmName].needRAM;
			server.serverVMSet[serID].insert({ VMid, 0 });
			server.updatVmSourceOrder(info[vmName], serID, true);
		}
	}
	else {
		if (server.myServerSet[serID].bIdleCPU < info[vmName].needCPU \
			|| server.myServerSet[serID].bIdleRAM < info[vmName].needRAM) {
			cout << "b node error" << endl;
			return 2;
		}
		else {
			server.myServerSet[serID].bIdleCPU -= info[vmName].needCPU;
			server.myServerSet[serID].bIdleRAM -= info[vmName].needRAM;
			server.serverVMSet[serID].insert({ VMid, 1 });
			server.updatVmSourceOrder(info[vmName], serID, true);
		}
	}

	sEachWorkingVM oneVM;
	oneVM.vmName = vmName;
	oneVM.serverID = serID;
	oneVM.node = node;
	if (!workingVmSet.count(VMid)) {
		workingVmSet.insert(std::make_pair(VMid, oneVM));
	}
	else {
		cout << "虚拟机id冲突！" << endl;
		return 3;
	}

	sDeployItem oneDeploy;
	oneDeploy.serID = serID;
	oneDeploy.isSingle = true;
	oneDeploy.node = node;

	deployRecord[iDay].insert(make_pair(VMid, oneDeploy));

	return 0;
}

int cVM::deploy(cServer &server, int iDay, string VMid, string vmName, int serID) {
	/*
	* Fn: 双节点部署
	* 	- 因为要求输出的顺序按照输入虚拟机请求的顺序，所以这个函数的调用必须按照add请求的顺序
	*	- 其他参考 单节点部署 函数
	*/
	if (!info[vmName].nodeStatus) {
		cout << "单节点虚拟机分配两个节点，分配失败" << endl;
		return 1;
	}

	if (serID == 126) {
		int a = 1;
	}

	if (server.myServerSet[serID].aIdleCPU<info[vmName].needCPU / 2 \
		|| server.myServerSet[serID].bIdleCPU<info[vmName].needCPU / 2 \
		|| server.myServerSet[serID].aIdleRAM < info[vmName].needRAM / 2 \
		|| server.myServerSet[serID].bIdleRAM < info[vmName].needRAM / 2) { 
		cout << "服务器资源不够，分配失败" << endl;
		return 2;
	}
	else {
		server.myServerSet[serID].aIdleCPU -= info[vmName].needCPU / 2;
		server.myServerSet[serID].aIdleRAM -= info[vmName].needRAM / 2;
		server.myServerSet[serID].bIdleCPU -= info[vmName].needCPU / 2;
		server.myServerSet[serID].bIdleRAM -= info[vmName].needRAM / 2;
		server.serverVMSet[serID].insert({ VMid, 2});
		server.updatVmSourceOrder(info[vmName], serID, true);
	}

	sEachWorkingVM oneVM;
	oneVM.vmName = vmName;
	oneVM.serverID = serID;
	if (!workingVmSet.count(VMid)) {
		workingVmSet.insert(std::make_pair(VMid, oneVM));
	}
	else {
		cout << "虚拟机id冲突！" << endl;
		return 3;
	}

	sDeployItem oneDeploy;
	oneDeploy.serID = serID;
	oneDeploy.isSingle = false;

	deployRecord[iDay].insert(make_pair(VMid, oneDeploy));

	return 0;
}

void cVM::transfer(cServer &server, int iDay, string VMid, int serID, bool node) {
	/*
	* Fn: 单节点迁移，出入参数错误检测遗留，包括5%。的要求等
	*/
	string vmName = workingVmSet[VMid].vmName;
	int lastServerID = workingVmSet[VMid].serverID;
	bool lastServerNode = workingVmSet[VMid].node;
	int occupyCPU = info[vmName].needCPU;
	int occupyRAM = info[vmName].needRAM;

	// 更改该虚拟机现在的位置
	workingVmSet[VMid].serverID = serID;
	workingVmSet[VMid].node = node;

	//////////////////////////用于检测错误///////////////////////////////
	sServerItem outServer = server.info[server.myServerSet[lastServerID].serName];
	sServerItem inServer = server.info[server.myServerSet[serID].serName];
	///////////////////////////////////////////////////////////////

	// 恢复前一个服务器的资源
	if (lastServerNode == true) { // A node
		server.myServerSet[lastServerID].aIdleCPU += occupyCPU;
		server.myServerSet[lastServerID].aIdleRAM += occupyRAM;
		if (server.myServerSet[lastServerID].aIdleCPU > outServer.totalCPU / 2 ||
			server.myServerSet[lastServerID].aIdleRAM > outServer.totalRAM / 2) {
			cout << "migrate a node error" << endl;
		}
	}
	else {
		server.myServerSet[lastServerID].bIdleCPU += occupyCPU;
		server.myServerSet[lastServerID].bIdleRAM += occupyRAM;
		if (server.myServerSet[lastServerID].bIdleCPU > outServer.totalCPU / 2 ||
			server.myServerSet[lastServerID].bIdleRAM > outServer.totalRAM / 2) {
			cout << "migrate b node error" << endl;
		}
	}
	server.serverVMSet[lastServerID].erase(VMid);    // 从迁出的服务器中删除该虚拟机

	// 占据新服务器资源
	if (node == true) {
		server.myServerSet[serID].aIdleCPU -= occupyCPU;
		server.myServerSet[serID].aIdleRAM -= occupyRAM;
		if (server.myServerSet[serID].aIdleCPU < 0 || server.myServerSet[serID].aIdleRAM < 0) {
			cout << "exit a node" << endl;
		}
		server.serverVMSet[serID].insert({ VMid, 0 });    // 往迁入的服务器中加入该虚拟机
	}
	else {
		server.myServerSet[serID].bIdleCPU -= occupyCPU;
		server.myServerSet[serID].bIdleRAM -= occupyRAM;
		if (server.myServerSet[serID].bIdleCPU < 0 || server.myServerSet[serID].bIdleRAM < 0) {
			cout << "exit b node" << endl;
		}
		server.serverVMSet[serID].insert({ VMid, 1 });    // 往迁入的服务器中加入该虚拟机
	}

	// 迁移条目更新
	sTransVmItem oneTrans;
	oneTrans.vmID = VMid;
	oneTrans.serverID = serID;
	oneTrans.node = node;
	oneTrans.isSingle = true;
	transVmRecord[iDay].push_back(oneTrans);
}

void cVM::transfer(cServer &server, int iDay, string VMid, int serID) {
	/*
	* Fn: 双节点迁移，出入参数错误检测遗留
	*/
	string vmName = workingVmSet[VMid].vmName;
	int lastServerID = workingVmSet[VMid].serverID;
	int occupyCPU = info[vmName].needCPU;
	int occupyRAM = info[vmName].needRAM;

	workingVmSet[VMid].serverID = serID;

	if (serID == 126 || lastServerID == 126) {
		int a = 1;
	}

	//////////////////////////////////////////////////////////////
	sServerItem outServer = server.info[server.myServerSet[lastServerID].serName];
	//////////////////////////////////////////////////////////

	// 恢复前一个服务器的资源
	server.myServerSet[lastServerID].aIdleCPU += occupyCPU / 2;
	server.myServerSet[lastServerID].aIdleRAM += occupyRAM / 2;
	server.myServerSet[lastServerID].bIdleCPU += occupyCPU / 2;
	server.myServerSet[lastServerID].bIdleRAM += occupyRAM / 2;
	server.serverVMSet[lastServerID].erase(VMid);    // 从迁出的服务器中移除虚拟机

	if (server.myServerSet[lastServerID].aIdleCPU > outServer.totalCPU / 2 ||
		server.myServerSet[lastServerID].bIdleCPU > outServer.totalCPU / 2 ||
		server.myServerSet[lastServerID].aIdleRAM > outServer.totalRAM / 2 ||
		server.myServerSet[lastServerID].bIdleRAM > outServer.totalRAM / 2) {
		cout << "double migrate error" << endl;
	}

	// 占据新服务器资源
	server.myServerSet[serID].aIdleCPU -= occupyCPU / 2;
	server.myServerSet[serID].aIdleRAM -= occupyRAM / 2;
	server.myServerSet[serID].bIdleCPU -= occupyCPU / 2;
	server.myServerSet[serID].bIdleRAM -= occupyRAM / 2;
	server.serverVMSet[serID].insert({ VMid, 2 });   // 从迁入的服务器中加入虚拟机

	if (server.myServerSet[serID].aIdleCPU < 0 ||
		server.myServerSet[serID].bIdleCPU < 0 ||
		server.myServerSet[serID].aIdleRAM < 0 ||
		server.myServerSet[serID].bIdleRAM < 0 ) {
		cout << "double exit error" << endl;
	}

	// 迁移条目更新
	sTransVmItem oneTrans;
	oneTrans.vmID = VMid;
	oneTrans.serverID = serID;
	oneTrans.isSingle = false;
	transVmRecord[iDay].push_back(oneTrans);
}

bool cVM::isDouble(string vmName) {
	return info[vmName].nodeStatus;
}

int cVM::reqCPU(string vmName) {
	return info[vmName].needCPU;
}

int cVM::reqRAM(string vmName) {
	return info[vmName].needRAM;
}

int cVM::deleteVM(string vmID, cServer& server) {
	if (!workingVmSet.count(vmID)) {
		cout << "不能删除不存在的服务器" << endl;
		return -1;
	}


	string vmName = workingVmSet[vmID].vmName;
	bool doubleStatus = isDouble(vmName);
	int serID = workingVmSet[vmID].serverID;
	int reqCPUs = reqCPU(vmName);
	int reqRAMs = reqRAM(vmName);

	// 恢复服务器资源
	if (doubleStatus) {
		server.myServerSet[serID].aIdleCPU += reqCPUs / 2;
		server.myServerSet[serID].bIdleCPU += reqCPUs / 2;
		server.myServerSet[serID].aIdleRAM += reqRAMs / 2;
		server.myServerSet[serID].bIdleRAM += reqRAMs / 2;
	}
	else {
		if (workingVmSet[vmID].node) { // A
			server.myServerSet[serID].aIdleCPU += reqCPUs;
			server.myServerSet[serID].aIdleRAM += reqRAMs;
		}
		else { // B
			server.myServerSet[serID].bIdleCPU += reqCPUs;
			server.myServerSet[serID].bIdleRAM += reqRAMs;
		}
	}

	workingVmSet.erase(vmID);
	return 0;
}