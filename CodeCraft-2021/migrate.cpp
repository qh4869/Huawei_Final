#include "migrate.h"

// 通过多次迭代来实现迁移
void migrateVM_3(cServer &server, cVM &VM, int whichDay, unordered_map<string, int> &dayWorkingVM,
	int vmTotalNum, unordered_map<int, sMyEachServer> &delSerSet, vector<double> &args) {

	int migrateNum = vmTotalNum * 5 / 1000;   // 迁移数量
	if (migrateNum <= 0) { return; }
	int cntMig = 0;   // 迁移的服务器数量统计	
	unordered_set<int> emptySer;    // 空的服务器集合，该服务器不可作为迁入服务器

	for (int i = 0; i < 2; i++) {
		subMigrateVM(server, VM, whichDay, delSerSet, dayWorkingVM, emptySer, args, cntMig, migrateNum);
		cout << emptySer.size() << endl;
	}
	 

}


void subMigrateVM(cServer &server, cVM &VM, int whichDay, unordered_map<int, sMyEachServer> &delSerSet,
	unordered_map<string, int> &dayWorkingVM, unordered_set<int> &emptySer, vector<double> &args, 
	int &cntMig, int &migrateNum) {

	int serID;    // 迁出的serID
	int vmNum;    // 服务器的虚拟机数量
	string vmID;  // 服务器ID
	sEachWorkingVM workVM;   // 工作中的虚拟机
	sVmItem requestVM;   // 记录虚拟机需要的资源
	sServerItem myServer;
	unordered_map<string, int> tempSerVmSet;

	for (int i = 0; i < (int)server.vmSourceOrder.size(); i++) {   // 遍历一次服务器，把可以迁出的迁出

		serID = server.vmSourceOrder[i].first;   // 迁出的服务器id
		if (!server.isOpen(serID)) {      // 空的服务器就不迁移了，直接继续
			emptySer.insert(serID);
			continue;
		}

		tempSerVmSet = server.serverVMSet[serID];
		for (auto ite = tempSerVmSet.begin(); ite != tempSerVmSet.end(); ite++) {
			if (cntMig >= migrateNum) {    // 达到数量限制，直接退出
				return;
			}
			vmID = ite->first;   // 取出虚拟机ID
			if (dayWorkingVM.count(vmID) == 1) {
				break;    // 当天的虚拟机暂时不处理
			}

			workVM = VM.workingVmSet[vmID];
			requestVM = VM.info[workVM.vmName];
			myServer = chooseFirstServer(server, requestVM, VM, i, delSerSet, emptySer, args);
			if (myServer.hardCost == -1) {   // 表示找到了合适的服务器
				cntMig++;
				if (requestVM.nodeStatus) {  // true : double node
					VM.transfer(server, whichDay, vmID, myServer.buyID);
				}
				else {   // false : single node
					VM.transfer(server, whichDay, vmID, myServer.buyID, myServer.node);
				}

				if (delSerSet.count(myServer.buyID) == 1) {
					updateDelSerSet(delSerSet, requestVM, myServer.node, myServer.buyID, true);  // 添加操作
				}
				else if (delSerSet.count(serID) == 1) {
					updateDelSerSet(delSerSet, requestVM, workVM.node, serID, false);   // 删除操作
				}
				server.updatVmSourceOrder(requestVM, serID, false, args);      // 删除操作
				server.updatVmSourceOrder(requestVM, myServer.buyID, true, args);   // 添加操作
			}
			else {   // 没有找到合适的服务器
				break;
			}

		}

	}

}


// 选择第一次选到的服务器
sServerItem chooseFirstServer(cServer &server, sVmItem &requestVM, cVM &VM, int index,
	unordered_map<int, sMyEachServer> &delSerSet, unordered_set<int> &emptySer, vector<double> &args) {

	sServerItem myServer;
	myServer.hardCost = 1;   // hardCost = 1表示没有找到合适的服务器

	if (server.myServerSet.size() > 0) {    // 有服务器才开始找

		sMyEachServer tempServer;
		int restCPU;      // 装入虚拟机后剩余的CPU
		int restRAM;      // 装入虚拟机后剩余的RAM
		int minValue = INT_MAX;   // 记录最小值
		int serID;    // 迁入的服务器ID
		int tempValue;
		double first = (double)server.vmSourceOrder[index].second;   // 迁出服务器的资源数
		double second;   // 迁入服务器的资源数

		for (int i = index + 1; i < server.vmSourceOrder.size(); i++) {    // 从后往前找

			second = (double)server.vmSourceOrder[i].second;
			// 当占用资源数是迁出的N倍时结束（这时可能更容易超出限制，所以直接退出）
			if (second / first < args[1]) { break; }

			serID = server.vmSourceOrder[i].first;
			if (emptySer.count(serID) == 1) { continue; }
			if (delSerSet.count(serID) == 1) {    // 如果该服务器在当天有删除操作
				tempServer = delSerSet[serID];    
			}
			else {
				tempServer = server.myServerSet[serID];
			}

			if (!requestVM.nodeStatus) {   // single node
				if (tempServer.aIdleCPU >= requestVM.needCPU && tempServer.aIdleRAM >= requestVM.needRAM) {    // a节点
					tempServer = server.myServerSet[serID];   // 最后算比较值的时候还是得用myServerSet里的值
					restCPU = tempServer.aIdleCPU - requestVM.needCPU;
					restRAM = tempServer.aIdleRAM - requestVM.needRAM;
					tempValue = restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3];
					if (tempValue < minValue) {
						minValue = tempValue;
						myServer.energyCost = -1;
						myServer.hardCost = -1;
						myServer.buyID = serID;   // 记录该服务器
						myServer.node = true;   // 返回true表示a 节点
					}
				}

				if (delSerSet.count(serID) == 1) {   // 该服务器在当天有删除操作
					tempServer = delSerSet[serID];
				}
				else {  // 表示这台服务器当天没有删除操作
					tempServer = server.myServerSet[serID];  
				}

				// 两个节点都要查看，看看放哪个节点更合适
				if (tempServer.bIdleCPU >= requestVM.needCPU && tempServer.bIdleRAM >= requestVM.needRAM) {  // b 节点
					tempServer = server.myServerSet[serID];   // 最后算比较值的时候还是得用myServerSet里的值
					restCPU = tempServer.bIdleCPU - requestVM.needCPU;
					restRAM = tempServer.bIdleRAM - requestVM.needRAM;
					tempValue = restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3];
					if (tempValue < minValue) {
						minValue = tempValue;
						myServer.energyCost = -1;
						myServer.hardCost = -1;
						myServer.buyID = serID;   // 记录服务器
						myServer.node = false;   // 返回false表示b 节点
					}
				}
			}
			else {   // double node
				if (tempServer.aIdleCPU >= requestVM.needCPU / 2 && tempServer.aIdleRAM >= requestVM.needRAM / 2
					&& tempServer.bIdleCPU >= requestVM.needCPU / 2 && tempServer.bIdleRAM >= requestVM.needRAM / 2) {
					tempServer = server.myServerSet[serID];   // 最后算比较值的时候还是得用myServerSet里的值
					restCPU = tempServer.aIdleCPU + tempServer.bIdleCPU - requestVM.needCPU;
					restRAM = tempServer.aIdleRAM + tempServer.bIdleRAM - requestVM.needRAM;
					tempValue = restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3];
					if (tempValue < minValue) {
						minValue = tempValue;
						myServer.energyCost = -1;
						myServer.hardCost = -1;
						myServer.buyID = serID;   // 记录服务器
					}
				}
			}

			if (myServer.hardCost == -1) {   // 找到了合适的则退出
				return myServer;
			}

		}

	}

	return myServer;

}