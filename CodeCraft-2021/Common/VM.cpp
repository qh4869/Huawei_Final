#include "VM.h"

void cVM::deploy(cServer &server, int iDay, string VMid, string vmName, int serID, bool node) {
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

void cVM::deploy(cServer &server, int iDay, string VMid, string vmName, int serID) {
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

void cVM::transfer(cServer &server, int iDay, string VMid, int serID, bool node) {
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

	/*占用新服务器的资源*/
	if (node == true) { // node A
		if (server.myServerSet[serID].aIdleCPU < occupyCPU \
			|| server.myServerSet[serID].aIdleRAM < occupyRAM) {
			cout << "迁移目标虚拟机资源不够（nodep a）" << endl;
			throw "resource not enough";
		}
		else {
			server.myServerSet[serID].aIdleCPU -= occupyCPU;
			server.myServerSet[serID].aIdleRAM -= occupyRAM;
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

void cVM::transfer(cServer &server, int iDay, string VMid, int serID) {
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

	/*占用新服务器的资源*/
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
	}

	workingVmSet[VMid].serverID = serID;

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
}

/******** 决赛新增内容 *******/
void cVM::updateRequest(int iDay, cRequests &request) {

	string inputStr;
	vector<sRequestItem> tempItem = request.info[iDay];
	vector<int> dayCompQuote;
	int count = 0;

	for (auto &req : tempItem) {
		if (req.type) {  // true : add
			cin >> inputStr;
			inputStr.erase(inputStr.begin()); // 删除左括号
			inputStr.erase(inputStr.end() - 1); // 逗号
			if (inputStr == "0") {   // 没有抢到该虚拟机
				request.info[iDay].erase(request.info[iDay].begin() + count);  // 删除
				request.numEachDay[iDay]--;  // 当天请求数-1
				lostVmSet.insert(req.vmID);   // 加入丢失集合
			}
			else  // 抢到该虚拟机
				count++;
			/* 获取对手报价 */
			cin >> inputStr;  
			inputStr.erase(inputStr.end() - 1); // 逗号
			dayCompQuote.push_back(atoi(inputStr.c_str()));
		}
		else {  // delete
			if (lostVmSet.count(req.vmID) == 1) {  // 没获得该虚拟机，不需要删除操作
				request.info[iDay].erase(request.info[iDay].begin() + count);  // 删除
				request.numEachDay[iDay]--;  // 当天请求数-1
			}
			else
				count++;
		}

	}
	compQuote.push_back(dayCompQuote);
}

void cVM::setQuote(cRequests &request, int iDay) {
	vector<int> dayQuote;  // 当天的报价
	double ratio = 0.8;  // 折扣
	for (auto &req : request.info[iDay]) {
		if (req.type)  // true : add
			dayQuote.push_back(req.quote * ratio);
	}
	quote.push_back(dayQuote);
}