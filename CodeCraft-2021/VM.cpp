#include "VM.h"

int cVM::deploy(cServer &server, int iDay, string VMid, string vmName, int serID, bool node){
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
	if(info[vmName].nodeStatus) {
		cout << "双节点虚拟机不能指定节点类型，分配失败" << endl;
		return 1;
	}

	// 判断资源是否够分
	if (node==true) { // a 节点部署
		if (server.myServerSet[serID].aIdleCPU < info[vmName].needCPU \
			|| server.myServerSet[serID].aIdleRAM < info[vmName].needRAM) {
			cout << "服务器资源不够，分配失败" << endl;
			return 2;
		}
		else {
			server.myServerSet[serID].aIdleCPU -= info[vmName].needCPU;
			server.myServerSet[serID].aIdleRAM -= info[vmName].needRAM;
		}
	}
	else {
		if (server.myServerSet[serID].bIdleCPU < info[vmName].needCPU \
			|| server.myServerSet[serID].bIdleRAM < info[vmName].needRAM) {
			cout << "服务器资源不够，分配失败" << endl;
			return 2;
		}
		else {
			server.myServerSet[serID].bIdleCPU -= info[vmName].needCPU;
			server.myServerSet[serID].bIdleRAM -= info[vmName].needRAM;
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

	deployRecord[iDay].push_back(oneDeploy);

	return 0;
}

int cVM::deploy(cServer &server, int iDay, string VMid, string vmName, int serID) {
/*
* Fn: 双节点部署
* 	- 因为要求输出的顺序按照输入虚拟机请求的顺序，所以这个函数的调用必须按照add请求的顺序
*	- 其他参考 单节点部署 函数
*/
	if(!info[vmName].nodeStatus) {
		cout << "单节点虚拟机分配两个节点，分配失败" << endl;
		return 1;
	}

	if (server.myServerSet[serID].aIdleCPU<info[vmName].needCPU/2 \
		|| server.myServerSet[serID].bIdleCPU<info[vmName].needCPU/2 \
		|| server.myServerSet[serID].aIdleRAM < info[vmName].needRAM/2 \
		|| server.myServerSet[serID].bIdleRAM < info[vmName].needRAM/2) {
		cout << "服务器资源不够，分配失败" << endl;
			return 2;
	}
	else {
		server.myServerSet[serID].aIdleCPU -= info[vmName].needCPU/2;
		server.myServerSet[serID].aIdleRAM -= info[vmName].needRAM/2;
		server.myServerSet[serID].bIdleCPU -= info[vmName].needCPU/2;
		server.myServerSet[serID].bIdleRAM -= info[vmName].needRAM/2;
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
	deployRecord[iDay].push_back(oneDeploy);

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

	// 恢复前一个服务器的资源
	if (lastServerNode==true) { // A node
		server.myServerSet[lastServerID].aIdleCPU += occupyCPU;
		server.myServerSet[lastServerID].aIdleRAM += occupyRAM;
	}
	else {
		server.myServerSet[lastServerID].bIdleCPU += occupyCPU;
		server.myServerSet[lastServerID].bIdleRAM += occupyRAM;
	}

	// 占据新服务器资源
	if (node==true) {
		server.myServerSet[serID].aIdleCPU -= occupyCPU;
		server.myServerSet[serID].aIdleRAM -= occupyRAM;
	}
	else {
		server.myServerSet[serID].bIdleCPU -= occupyCPU;
		server.myServerSet[serID].bIdleRAM -= occupyRAM;
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

	// 恢复前一个服务器的资源
	server.myServerSet[lastServerID].aIdleCPU += occupyCPU/2;
	server.myServerSet[lastServerID].aIdleRAM += occupyRAM/2;
	server.myServerSet[lastServerID].bIdleCPU += occupyCPU/2;
	server.myServerSet[lastServerID].bIdleRAM += occupyRAM/2;

	// 占据新服务器资源
	server.myServerSet[serID].aIdleCPU -= occupyCPU/2;
	server.myServerSet[serID].aIdleRAM -= occupyRAM/2;
	server.myServerSet[serID].bIdleCPU -= occupyCPU/2;
	server.myServerSet[serID].bIdleRAM -= occupyRAM/2;

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

void cVM::deleteVM(string vmID, cServer& server) {
	string vmName = workingVmSet[vmID].vmName;
	bool doubleStatus = isDouble(vmName);
	int serID = workingVmSet[vmID].serverID;
	int reqCPUs = reqCPU(vmName);
	int reqRAMs = reqRAM(vmName);

	// 恢复服务器资源
	if (doubleStatus) {
		server.myServerSet[serID].aIdleCPU += reqCPUs/2;
		server.myServerSet[serID].bIdleCPU += reqCPUs/2;
		server.myServerSet[serID].aIdleRAM += reqRAMs/2;
		server.myServerSet[serID].bIdleRAM += reqRAMs/2;
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
}

void cVM::predp(string vmID, bool isNew, int serID, string vmName, cServer &server, bool node) {
/* Fn: 预部署 单节点
*/
	int reqCPU = info[vmName].needCPU;
	int reqRAM = info[vmName].needRAM;

	// 减少资源 (preSvSet & mySerCopy)
	if (isNew) {
		if (node) {
			server.preSvSet[serID].aIdleCPU -= reqCPU;
			server.preSvSet[serID].aIdleRAM -= reqRAM;
		}
		else {
			server.preSvSet[serID].bIdleCPU -= reqCPU;
			server.preSvSet[serID].bIdleRAM -= reqRAM;
		}
	}
	else {
		if (node) {
			server.mySerCopy[serID].aIdleCPU -= reqCPU;
			server.mySerCopy[serID].aIdleRAM -= reqRAM;
		}
		else {
			server.mySerCopy[serID].bIdleCPU -= reqCPU;
			server.mySerCopy[serID].bIdleRAM -= reqRAM;
		}
	}

	// 输入数据结构
	sPreDeployItem onePreDeploy;

	onePreDeploy.isNew = isNew;
	onePreDeploy.serID = serID;
	onePreDeploy.node = node;
	onePreDeploy.vmName = vmName;

	pair<string, sPreDeployItem> res(vmID, onePreDeploy);
	preDeploy.insert(res);
}

void cVM::predp(string vmID, bool isNew, int serID, string vmName, cServer &server) {
/* Fn: 预部署 双节点
*/
	int reqCPU = info[vmName].needCPU;
	int reqRAM = info[vmName].needRAM;
	
	if (isNew) {
		server.preSvSet[serID].aIdleCPU -= reqCPU/2;
		server.preSvSet[serID].bIdleCPU -= reqCPU/2;
		server.preSvSet[serID].aIdleRAM -= reqRAM/2;
		server.preSvSet[serID].bIdleRAM -= reqRAM/2;
	}
	else {
		server.mySerCopy[serID].aIdleCPU -= reqCPU/2;
		server.mySerCopy[serID].bIdleCPU -= reqCPU/2;
		server.mySerCopy[serID].aIdleRAM -= reqRAM/2;
		server.mySerCopy[serID].bIdleRAM -= reqRAM/2;
	}

	sPreDeployItem onePreDeploy;

	onePreDeploy.isNew = isNew;
	onePreDeploy.serID = serID;
	onePreDeploy.vmName = vmName;

	pair<string, sPreDeployItem> res(vmID, onePreDeploy);
	preDeploy.insert(res);
}

void cVM::preDltVM(string vmID) {
	preDel.push_back(vmID);
}

int cVM::postDpWithDel(cServer &server, const cRequests &request, int iDay) {
	int postid;
	int bugID;
	string vmID;
	string vmName;
	bool isdouble;
	bool isNew;
	int preid;
	bool node;

	for (int iTerm=0; iTerm<request.numEachDay[iDay]; iTerm++) {
		if (request.info[iDay][iTerm].type) { //
			vmID = request.info[iDay][iTerm].vmID;
			vmName = preDeploy[vmID].vmName;
			isdouble = isDouble(vmName);
			isNew = preDeploy[vmID].isNew;
			preid = preDeploy[vmID].serID;
			node = preDeploy[vmID].node;

			// id 映射
			if (isNew)
				postid = server.idMap[preid];
			else
				postid = preid;

			if (isdouble)
				bugID = deploy(server, iDay, vmID, vmName, postid);
			else
				bugID = deploy(server, iDay, vmID, vmName, postid, node);

			if (bugID) {
				cout << "部署失败" << bugID << endl;
				return -1;
			}
		}
		else {
			vmID = request.info[iDay][iTerm].vmID;
			deleteVM(vmID, server);
		}
	}

	server.mySerCopy = server.myServerSet;
	server.idMap.clear();

	return 0;
}