#include "SSP_Mig_VM.h"

void cSSP_Mig_VM::deploy(cSSP_Mig_Server &server, int iDay, string VMid, string vmName, int serID, bool node) {
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
		throw "node type error in deploying";
	}

	// 判断资源是否够分
	if (node == true) { // a 节点部署
		if (server.myServerSet[serID].aIdleCPU < info[vmName].needCPU \
			|| server.myServerSet[serID].aIdleRAM < info[vmName].needRAM) {
			cout << "服务器资源不够，分配失败" << endl;
			throw "not enough resource";
		}
		else {
			server.myServerSet[serID].aIdleCPU -= info[vmName].needCPU;
			server.myServerSet[serID].aIdleRAM -= info[vmName].needRAM;
			server.updatVmSourceOrder(info[vmName].needCPU, info[vmName].needRAM, serID, true);
			server.updatVmTarOrder(info[vmName].needCPU, info[vmName].needRAM, 0, 0, serID, true);
			server.serverVMSet[serID].insert({ VMid, 0 });
		}
	}
	else {
		if (server.myServerSet[serID].bIdleCPU < info[vmName].needCPU \
			|| server.myServerSet[serID].bIdleRAM < info[vmName].needRAM) {
			cout << "服务器资源不够，分配失败" << endl;
			throw "not enough resource";
		}
		else {
			server.myServerSet[serID].bIdleCPU -= info[vmName].needCPU;
			server.myServerSet[serID].bIdleRAM -= info[vmName].needRAM;
			server.updatVmSourceOrder(info[vmName].needCPU, info[vmName].needRAM, serID, true);
			server.updatVmTarOrder(0, 0, info[vmName].needCPU, info[vmName].needRAM, serID, true);
			server.serverVMSet[serID].insert({ VMid, 1 });
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
		throw "VM id conflict";
	}

	sDeployItem oneDeploy;
	oneDeploy.serID = serID;
	oneDeploy.isSingle = true;
	oneDeploy.node = node;

	deployRecord[iDay].insert(make_pair(VMid, oneDeploy));
}

void cSSP_Mig_VM::deploy(cSSP_Mig_Server &server, int iDay, string VMid, string vmName, int serID) {
	/*
	* Fn: 双节点部署
	* 	- 因为要求输出的顺序按照输入虚拟机请求的顺序，所以这个函数的调用必须按照add请求的顺序
	*	- 其他参考 单节点部署 函数
	*/
	if (!info[vmName].nodeStatus) {
		cout << "单节点虚拟机分配两个节点，分配失败" << endl;
		throw "node type error in deploying";
	}

	if (server.myServerSet[serID].aIdleCPU<info[vmName].needCPU / 2 \
		|| server.myServerSet[serID].bIdleCPU<info[vmName].needCPU / 2 \
		|| server.myServerSet[serID].aIdleRAM < info[vmName].needRAM / 2 \
		|| server.myServerSet[serID].bIdleRAM < info[vmName].needRAM / 2) {
		cout << "服务器资源不够，分配失败" << endl;
		throw "not enough resource";
	}
	else {
		server.myServerSet[serID].aIdleCPU -= info[vmName].needCPU / 2;
		server.myServerSet[serID].aIdleRAM -= info[vmName].needRAM / 2;
		server.myServerSet[serID].bIdleCPU -= info[vmName].needCPU / 2;
		server.myServerSet[serID].bIdleRAM -= info[vmName].needRAM / 2;
		server.serverVMSet[serID].insert({ VMid, 2 });
		server.updatVmSourceOrder(info[vmName].needCPU, info[vmName].needRAM, serID, true);
		server.updatVmTarOrder(info[vmName].needCPU / 2, info[vmName].needRAM / 2, info[vmName].needCPU / 2, info[vmName].needRAM / 2,
			serID, true);
	}

	sEachWorkingVM oneVM;
	oneVM.vmName = vmName;
	oneVM.serverID = serID;
	if (!workingVmSet.count(VMid)) {
		workingVmSet.insert(std::make_pair(VMid, oneVM));
	}
	else {
		cout << "虚拟机id冲突！" << endl;
		throw "VM id conflict";
	}

	sDeployItem oneDeploy;
	oneDeploy.serID = serID;
	oneDeploy.isSingle = false;
	deployRecord[iDay].insert(make_pair(VMid, oneDeploy));
}

void cSSP_Mig_VM::transfer(cSSP_Mig_Server &server, int iDay, string VMid, int serID, bool node) {
	/*
	* Fn: 单节点迁移，出入参数错误检测遗留，包括5%。的要求等
	*/
	string vmName = workingVmSet[VMid].vmName;
	int lastServerID = workingVmSet[VMid].serverID;
	bool lastServerNode = workingVmSet[VMid].node;
	int occupyCPU = info[vmName].needCPU;
	int occupyRAM = info[vmName].needRAM;
	bool vmIsDouble = isDouble(vmName);

	/*检查VM是否已经被部署*/
	if (!workingVmSet.count(VMid)) {
		cout << "待迁移的虚拟机没有被部署" << endl;
		throw "not deployed";
	}

	/*单双节点是否正确*/
	if (vmIsDouble) {
		cout << "双节点虚拟机不能单节点迁移" << endl;
		throw "not single";
	}

	// 恢复前一个服务器的资源
	if (lastServerNode == true) { // A node
		server.myServerSet[lastServerID].aIdleCPU += occupyCPU;
		server.myServerSet[lastServerID].aIdleRAM += occupyRAM;
	}
	else {
		server.myServerSet[lastServerID].bIdleCPU += occupyCPU;
		server.myServerSet[lastServerID].bIdleRAM += occupyRAM;
	}
	server.serverVMSet[lastServerID].erase(VMid);

	if (node == true) { // node A
		if (server.myServerSet[serID].aIdleCPU < occupyCPU \
			|| server.myServerSet[serID].aIdleRAM < occupyRAM) {
			cout << "迁移目标虚拟机资源不够（node a）" << endl;
			throw "resource not enough";
		}
		else {
			server.myServerSet[serID].aIdleCPU -= occupyCPU;
			server.myServerSet[serID].aIdleRAM -= occupyRAM;
			server.serverVMSet[serID].insert({ VMid, 0 });
		}
	}
	else { // node B
		if (server.myServerSet[serID].bIdleCPU < occupyCPU \
			|| server.myServerSet[serID].bIdleRAM < occupyRAM) {
			cout << "迁移目标虚拟机资源不够（node b）" << endl;
			throw "resource not enough";
		}
		else {
			server.myServerSet[serID].bIdleCPU -= occupyCPU;
			server.myServerSet[serID].bIdleRAM -= occupyRAM;
			server.serverVMSet[serID].insert({ VMid, 1 });
		}
	}

	// 更改该虚拟机现在的位置
	workingVmSet[VMid].serverID = serID;
	workingVmSet[VMid].node = node;

	// 迁移条目更新
	sTransVmItem oneTrans;
	oneTrans.vmID = VMid;
	oneTrans.serverID = serID;
	oneTrans.node = node;
	oneTrans.isSingle = true;
	transVmRecord[iDay].push_back(oneTrans);
}

void cSSP_Mig_VM::transfer(cSSP_Mig_Server &server, int iDay, string VMid, int serID) {
	/*
	* Fn: 双节点迁移，出入参数错误检测遗留
	*/
	string vmName = workingVmSet[VMid].vmName;
	int lastServerID = workingVmSet[VMid].serverID;
	int occupyCPU = info[vmName].needCPU;
	int occupyRAM = info[vmName].needRAM;
	bool vmIsDouble = isDouble(vmName);

	/*检查VM是否已经被部署*/
	if (!workingVmSet.count(VMid)) {
		cout << "待迁移的虚拟机没有被部署" << endl;
		throw "not deployed";
	}

	/*单双节点是否正确*/
	if (!vmIsDouble) {
		cout << "双节点虚拟机不能单节点迁移" << endl;
		throw "not single";
	}

	// 恢复前一个服务器的资源
	server.myServerSet[lastServerID].aIdleCPU += occupyCPU / 2;
	server.myServerSet[lastServerID].aIdleRAM += occupyRAM / 2;
	server.myServerSet[lastServerID].bIdleCPU += occupyCPU / 2;
	server.myServerSet[lastServerID].bIdleRAM += occupyRAM / 2;
	server.serverVMSet[lastServerID].erase(VMid);

	if (server.myServerSet[serID].aIdleCPU < occupyCPU / 2 \
		|| server.myServerSet[serID].aIdleRAM < occupyRAM / 2 \
		|| server.myServerSet[serID].bIdleCPU < occupyCPU / 2 \
		|| server.myServerSet[serID].bIdleRAM < occupyRAM / 2) {
		cout << "迁移目标虚拟机资源不够" << endl;
		throw "resource not enough";
	}
	else {
		// 占据新服务器资源
		server.myServerSet[serID].aIdleCPU -= occupyCPU / 2;
		server.myServerSet[serID].aIdleRAM -= occupyRAM / 2;
		server.myServerSet[serID].bIdleCPU -= occupyCPU / 2;
		server.myServerSet[serID].bIdleRAM -= occupyRAM / 2;
		server.serverVMSet[serID].insert({ VMid, 2 });
	}

	workingVmSet[VMid].serverID = serID;

	// 迁移条目更新
	sTransVmItem oneTrans;
	oneTrans.vmID = VMid;
	oneTrans.serverID = serID;
	oneTrans.isSingle = false;
	transVmRecord[iDay].push_back(oneTrans);
}

void cSSP_Mig_VM::deleteVM(string vmID, cSSP_Mig_Server& server) {
	if (!workingVmSet.count(vmID)) {
		cout << "不能删除不存在的服务器" << endl;
		throw "VM to be deleted does not exist";
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
		server.updatVmTarOrder(reqCPUs / 2, reqRAMs / 2, reqCPUs / 2, reqRAMs / 2, serID, false);
	}
	else {
		if (workingVmSet[vmID].node) { // A
			server.myServerSet[serID].aIdleCPU += reqCPUs;
			server.myServerSet[serID].aIdleRAM += reqRAMs;
			server.updatVmTarOrder(reqCPUs, reqRAMs, 0, 0, serID, false);
		}
		else { // B
			server.myServerSet[serID].bIdleCPU += reqCPUs;
			server.myServerSet[serID].bIdleRAM += reqRAMs;
			server.updatVmTarOrder(0, 0, reqCPUs, reqRAMs, serID, false);
		}
	}

	/*更新vmSourceOrder, targetOrder*/
	server.updatVmSourceOrder(reqCPUs, reqRAMs, serID, false);

	/*删除serverVMset*/
	server.serverVMSet[serID].erase(vmID);

	workingVmSet.erase(vmID);

	/*delCnt*/
	delCnt++;
}

bool cSSP_Mig_VM::isLocked(string vmID) {
/* Fn: 检查vm锁，如果被锁住就跳过，但是锁的值还是累计的
*/
	if (stopSetCnt.count(vmID) && stopSetCnt[vmID] >= stopTimes){
		stopSetCnt[vmID]++;
		return true;
	}
	return false;
}

void cSSP_Mig_VM::addLock(string vmID) {
	if (stopSetCnt.count(vmID))
		stopSetCnt.at(vmID)++;
	else
		stopSetCnt.insert({vmID, 1});
}