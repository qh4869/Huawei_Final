#include "MigrationPlus.h"

extern int xcnt;

//////////////////////////////////////主要的迁移函数/////////////////////////////////////////////////
// 一次迁移大量的虚拟机
void massMigrate(cSSP_Mig_Server &server, cSSP_Mig_VM &VM, int whichDay, unordered_map<string, int> &dayWorkingVM,
	unordered_map<int, sMyEachServer> &delSerSet, int &cntMig, int &migrateNum, vector<double> &args, cSSP_Mig_Request &request) {

// 	/*快速搜索清空缓存*/
	VM.saveGoal.clear();

	map<int, map<int, map<int, map<int, vector<int>>>>> empVmTarOrder;   // 用于记录空服务器的排序
	unordered_set<int> outSerIDSet;    // 迁出的服务器ID集合
	int outSerID;   // 迁出的服务器ID
	int needCPUa = 0, needCPUb = 0, needRAMa = 0, needRAMb = 0, minEnergy = 0;  // 记录需要的资源以及能耗
	cyt::sServerItem myServer, preServer;  // 当前的最优迁入服务器以及上一次的最优服务器
	sMyEachServer outSer;   // 迁出的服务器
	sServerItem outSerInfo;  // 迁出的服务器原始信息
	bool findNoEmpty = true;  // true表示能在非空集合中找到
	bool findEmpty = true;  // true表示能在空服务器中找到
	int empConut = 0, maxCtrl = 50;
	int maxBreakCount = INT_MAX, breakCnt = 0;

	for (int i = 0; i < (int)server.vmSourceOrder.size(); i++) {

		outSerID = server.vmSourceOrder[i].first;  // 迁出的服务器ID
		outSer = server.myServerSet[outSerID];   // 迁出的服务器

		/////////////////////////////////////////////////////////////////////////////////////////////
		//////////// sServerItem结构体增加了buyID,node以及serName字段 ////////////////////////////
		outSerInfo = server.info[outSer.serName];  // 迁出的服务器的原始信息
		/////////////////////////////////////////////////////////////////////////////////////////////

		if ((double)server.vmSourceOrder[i].second / (double)(outSerInfo.totalCPU + outSerInfo.totalRAM) > server.ratio) {
			continue;
		}

		// 没有可以迁出的资源则继续
		if (server.vmSourceOrder[i].second == 0) {
			/////////////////////////////下面这三个函数都是新加的/////////////////////////
			//server.deleteVmTar(outSerID);   // 删除空的服务器
			deleteEmpVmTar(server, empVmTarOrder, outSerID);  // 避免重复加入，先删除
			updateEmpVmTarOrder(server, empVmTarOrder, outSerID);  // 加入空的排序
			continue;
		}

		if (cntMig + (int)server.serverVMSet[outSerID].size() > migrateNum) {   // 达到最大容量，不能再继续下去了
			if (outSerIDSet.size() >= 1) {   // 表示还有未处理的数据，先处理
				for (auto &outSerID : outSerIDSet) {   // 找出所有这些可以迁移的服务器并迁移
					migrateSer(server, VM, outSerID, dayWorkingVM, empVmTarOrder, whichDay, delSerSet,
						preServer.buyID, args);
				}
			}
			// 可以尝试迁移单个虚拟机
			migrateSerVm(server, VM, outSerID, dayWorkingVM, empVmTarOrder, whichDay, i, delSerSet, args, cntMig, migrateNum);
			return; // 处理完之后直接退出
		}
		// 走到这里说明容量没有达到限制
		if (!findEmpty) {   // 表示不能从空的地方找到符合的服务器
			if (migrateSerVm(server, VM, outSerID, dayWorkingVM, empVmTarOrder, whichDay, i,
				delSerSet, args, cntMig, migrateNum)) {   // 表示有空的服务器了
				findEmpty = true;
				findNoEmpty = true;
				needCPUa = 0; needCPUb = 0; needRAMa = 0; needRAMb = 0;  // 清空数据
				minEnergy = 0;
				outSerIDSet.clear();
			}
			empConut++;
			breakCnt++;
			if (breakCnt > maxBreakCount) { return; }
			if (empConut >= maxCtrl) {
				findEmpty = true;
				findNoEmpty = true;
				needCPUa = 0; needCPUb = 0; needRAMa = 0; needRAMb = 0;  // 清空数据
				minEnergy = 0;
				outSerIDSet.clear();
				empConut = 0;
			}
			continue;   // 继续
		}
		outSer = server.myServerSet[outSerID];   // 迁出的服务器
		outSerInfo = server.info[outSer.serName];  // 迁出的服务器的原始信息
		needCPUa += (outSerInfo.totalCPU / 2 - outSer.aIdleCPU);   // 需要迁出的资源数量
		needCPUb += (outSerInfo.totalCPU / 2 - outSer.bIdleCPU);
		needRAMa += (outSerInfo.totalRAM / 2 - outSer.aIdleRAM);
		needRAMb += (outSerInfo.totalRAM / 2 - outSer.bIdleRAM);
		minEnergy += outSerInfo.energyCost;    // 可以节省的能耗

		if (findNoEmpty || findEmpty)
			VM.saveGoal.clear();

		// 可以在非空里找，并且能够找到
		if (findNoEmpty && migToNoEmptySer(server, needCPUa, needCPUb, needRAMa, needRAMb, args, delSerSet, outSerID,
			outSerIDSet, myServer, preServer, cntMig, VM, dayWorkingVM, whichDay, empVmTarOrder, minEnergy, i)) {
			breakCnt = 0;
			continue;
		}
		else {   // 这里可以优化
			findNoEmpty = false;   // 不能再非空里找
		}

		if (findEmpty && migToEmptySer(server, needCPUa, needCPUb, needRAMa, needRAMb, args, delSerSet, outSerID,
			outSerIDSet, myServer, preServer, cntMig, VM, dayWorkingVM, whichDay, empVmTarOrder, minEnergy, i, findNoEmpty)) {
			breakCnt = 0;
			continue;
		}
		else {   // 这里可以优化
			findEmpty = false;
		}

	}

}

void deleteEmpVmTar(cSSP_Mig_Server &server, map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, 
	int serID) {

	string serName = server.myServerSet[serID].serName;
	int totalCPU = server.info[serName].totalCPU;
	int totalRAM = server.info[serName].totalRAM;
	int aIdleCPU = totalCPU / 2;
	int aIdleRAM = totalRAM / 2;
	int bIdleCPU = totalCPU / 2;
	int bIdleRAM = totalRAM / 2;

	/*删除旧节点*/
	if (empVmTarOrder.count(aIdleCPU) && empVmTarOrder[aIdleCPU].count(aIdleRAM) \
		&& empVmTarOrder[aIdleCPU][aIdleRAM].count(bIdleCPU) && empVmTarOrder[aIdleCPU][aIdleRAM][bIdleCPU].count(bIdleRAM)) {
		vector<int> &serSet = empVmTarOrder[aIdleCPU][aIdleRAM][bIdleCPU][bIdleRAM];
		for (auto it = serSet.begin(); it != serSet.end(); it++) {
			if (*it == serID) {
				serSet.erase(it);
				break;
			}
		}
		if (serSet.empty()) {  // 删除空的节点
			empVmTarOrder[aIdleCPU][aIdleRAM][bIdleCPU].erase(bIdleRAM);
			if (empVmTarOrder[aIdleCPU][aIdleRAM][bIdleCPU].empty()) {
				empVmTarOrder[aIdleCPU][aIdleRAM].erase(bIdleCPU);
				if (empVmTarOrder[aIdleCPU][aIdleRAM].empty()) {
					empVmTarOrder[aIdleCPU].erase(aIdleRAM);
					if (empVmTarOrder[aIdleCPU].empty()) {
						empVmTarOrder.erase(aIdleCPU);
					}
				}
			}
		}
	}
}

void updateEmpVmTarOrder(cSSP_Mig_Server &server, map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder,
	int serID) {

	int aIdleCPU = server.myServerSet[serID].aIdleCPU;
	int aIdleRAM = server.myServerSet[serID].aIdleRAM;
	int bIdleCPU = server.myServerSet[serID].bIdleCPU;
	int bIdleRAM = server.myServerSet[serID].bIdleRAM;
	/*加入新结点*/
	empVmTarOrder[aIdleCPU][aIdleRAM][bIdleCPU][bIdleRAM].push_back(serID);
}

// 将一个服务器里的资源集体迁移
void migrateSer(cSSP_Mig_Server &server, cSSP_Mig_VM &VM, int outSerID, unordered_map<string, int> &dayWorkingVM,
	map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int whichDay,
	unordered_map<int, sMyEachServer> &delSerSet, int inSerID, vector<double> &args) {

	unordered_map<string, int> tempSerVmSet = server.serverVMSet[outSerID];  // 需要迁出的所有虚拟机集合
	string vmID;
	sEachWorkingVM workVM;   // 工作中的虚拟机信息
	sVmItem requestVM;   // 记录虚拟机需要的资源
	for (auto ite = tempSerVmSet.begin(); ite != tempSerVmSet.end(); ite++) {
		vmID = ite->first;
		workVM = VM.workingVmSet[vmID];
		requestVM = VM.info[workVM.vmName];
		if (requestVM.nodeStatus) {   // true : double node
			if (dayWorkingVM.count(vmID) == 1) {   // 如果是当天的虚拟机，则重新部署
				/////////////////////////////增加了redeploy函数///////////////////////////////
				VM.reDeploy(server, whichDay, vmID, workVM.vmName, inSerID, args);
				if (delSerSet.count(inSerID) == 1) {
					updateDelSerSet(delSerSet, requestVM, workVM.node, inSerID, true);  // 添加操作
				}
				else if (delSerSet.count(outSerID) == 1) {
					updateDelSerSet(delSerSet, requestVM, workVM.node, outSerID, false);   // 删除操作
				}
				continue;  // 部署完继续处理其他虚拟机
			}
			else {
				VM.transfer(server, whichDay, vmID, inSerID);
				server.updatVmTarOrder(requestVM.needCPU / 2, requestVM.needRAM / 2, requestVM.needCPU / 2, requestVM.needRAM / 2,
					outSerID, false);
				server.updatVmTarOrder(requestVM.needCPU / 2, requestVM.needRAM / 2, requestVM.needCPU / 2, requestVM.needRAM / 2,
					inSerID, true);
			}
		}
		else {    // false : single node
			if (dayWorkingVM.count(vmID) == 1) {    // 当天添加的虚拟机则重新部署
				VM.reDeploy(server, whichDay, vmID, workVM.vmName, inSerID, workVM.node, args); // 注意workVM.node
				if (delSerSet.count(inSerID) == 1) {
					updateDelSerSet(delSerSet, requestVM, workVM.node, inSerID, true);  // 添加操作
				}
				else if (delSerSet.count(outSerID) == 1) {
					updateDelSerSet(delSerSet, requestVM, workVM.node, outSerID, false);   // 删除操作
				}
				continue;   // 部署完继续处理其他虚拟机
			}
			else {
				VM.transfer(server, whichDay, vmID, inSerID, workVM.node);  // 注意workVM.node的值
				if (workVM.node) {   // node a							
					server.updatVmTarOrder(requestVM.needCPU, requestVM.needRAM, 0, 0, inSerID, true);
					server.updatVmTarOrder(requestVM.needCPU, requestVM.needRAM, 0, 0, outSerID, false);
				}
				else {   // node b
					server.updatVmTarOrder(0, 0, requestVM.needCPU, requestVM.needRAM, inSerID, true);
					server.updatVmTarOrder(0, 0, requestVM.needCPU, requestVM.needRAM, outSerID, false);
				}
			}
		}
		if (delSerSet.count(inSerID) == 1) {
			updateDelSerSet(delSerSet, requestVM, workVM.node, inSerID, true);  // 添加操作
		}
		else if (delSerSet.count(outSerID) == 1) {
			updateDelSerSet(delSerSet, requestVM, workVM.node, outSerID, false);   // 删除操作
		}
		server.updatVmSourceOrder(requestVM.needCPU, requestVM.needRAM, outSerID, false);      // 删除操作
		server.updatVmSourceOrder(requestVM.needCPU, requestVM.needRAM, inSerID, true);   // 添加操作
	}

	//server.deleteVmTar(outSerID);  // 虚拟机迁移结束之后，该服务器就空了
	deleteEmpVmTar(server, empVmTarOrder, outSerID);
	updateEmpVmTarOrder(server, empVmTarOrder, outSerID);   // 将该空的虚拟机加入空的排序里

}

// 将服务器里的虚拟机一个个尝试性的往外迁移
bool migrateSerVm(cSSP_Mig_Server &server, cSSP_Mig_VM &VM, int outSerID, unordered_map<string, int> &dayWorkingVM,
	map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int whichDay, int index,
	unordered_map<int, sMyEachServer> &delSerSet, vector<double> &args, int &cntMig, int migrateNum) {

	unordered_map<string, int> tempSerVmSet = server.serverVMSet[outSerID];  // 需要尝试迁出的所有虚拟机集合
	string vmID;
	sEachWorkingVM workVM;  // 工作中的虚拟机
	sVmItem requestVM;   // 记录虚拟机需要的资源
	cyt::sServerItem myServer;
	unordered_set<int> emptySer;

	for (auto ite = tempSerVmSet.begin(); ite != tempSerVmSet.end(); ite++) {
		xcnt++;
		if (cntMig >= migrateNum) // 达到数量限制，直接退出
			return false;
		vmID = ite->first;   // 取出虚拟机ID

		workVM = VM.workingVmSet[vmID];   // 取出工作中的虚拟机
		requestVM = VM.info[workVM.vmName];   // 取出该虚拟机的资源信息
		// myServer = chooseFirstServer(server, requestVM, VM, index, delSerSet, emptySer, args, vmID);
		myServer = bestFitMigrate(server, requestVM, VM, index, delSerSet, vmID);
		if (myServer.hardCost == -1) {   // 表示找到了合适的服务器
			cntMig++;
			if (requestVM.nodeStatus) {  // true : double node
				if (myServer.buyID == outSerID) {
					continue;
				}
				if (dayWorkingVM.count(vmID) == 1) {
					VM.reDeploy(server, whichDay, vmID, workVM.vmName, myServer.buyID, args);
					VM.saveGoal.clear();

					if (delSerSet.count(myServer.buyID) == 1) {
						updateDelSerSet(delSerSet, requestVM, myServer.node, myServer.buyID, true);  // 添加操作
					}
					else if (delSerSet.count(outSerID) == 1) {
						updateDelSerSet(delSerSet, requestVM, workVM.node, outSerID, false);   // 删除操作
					}
					continue;
				}

				VM.transfer(server, whichDay, vmID, myServer.buyID);
				VM.saveGoal.clear();

				server.updatVmTarOrder(requestVM.needCPU / 2, requestVM.needRAM / 2, requestVM.needCPU / 2, requestVM.needRAM / 2,
					outSerID, false);
				server.updatVmTarOrder(requestVM.needCPU / 2, requestVM.needRAM / 2, requestVM.needCPU / 2, requestVM.needRAM / 2,
					myServer.buyID, true);

			}
			else {   // false : single node
				if (myServer.buyID == outSerID && myServer.node == workVM.node) {
					continue;
				}
				if (dayWorkingVM.count(vmID) == 1) {
					VM.reDeploy(server, whichDay, vmID, workVM.vmName, myServer.buyID, myServer.node, args);
					VM.saveGoal.clear();

					if (delSerSet.count(myServer.buyID) == 1) {
						updateDelSerSet(delSerSet, requestVM, myServer.node, myServer.buyID, true);  // 添加操作
					}
					else if (delSerSet.count(outSerID) == 1) {
						updateDelSerSet(delSerSet, requestVM, workVM.node, outSerID, false);   // 删除操作
					}
					continue;
				}
				VM.transfer(server, whichDay, vmID, myServer.buyID, myServer.node);
				VM.saveGoal.clear();

				if (myServer.buyID == outSerID) {
					if (myServer.node) {  // 往a节点加
						server.updatVmTarOrder(requestVM.needCPU, requestVM.needRAM, -1 * requestVM.needCPU,
							-1 * requestVM.needRAM, myServer.buyID, true);
					}
					else {   // 往b里加
						server.updatVmTarOrder(-1 * requestVM.needCPU, -1 * requestVM.needRAM, requestVM.needCPU,
							requestVM.needRAM, myServer.buyID, true);
					}
				}
				else {
					if (myServer.node)  // node a
						server.updatVmTarOrder(requestVM.needCPU, requestVM.needRAM, 0, 0, myServer.buyID, true);
					else
						server.updatVmTarOrder(0, 0, requestVM.needCPU, requestVM.needRAM, myServer.buyID, true);
					if (workVM.node) // node a
						server.updatVmTarOrder(requestVM.needCPU, requestVM.needRAM, 0, 0, outSerID, false);
					else
						server.updatVmTarOrder(0, 0, requestVM.needCPU, requestVM.needRAM, outSerID, false);
				}
			}

			if (delSerSet.count(myServer.buyID) == 1) {
				updateDelSerSet(delSerSet, requestVM, myServer.node, myServer.buyID, true);  // 添加操作
			}
			else if (delSerSet.count(outSerID) == 1) {
				updateDelSerSet(delSerSet, requestVM, workVM.node, outSerID, false);   // 删除操作
			}
			server.updatVmSourceOrder(requestVM.needCPU, requestVM.needRAM, outSerID, false);      // 删除操作
			server.updatVmSourceOrder(requestVM.needCPU, requestVM.needRAM, myServer.buyID, true);   // 添加操作

		}
		else {   // 没有找到合适的服务器
			continue;
		}
	}

	if (!server.isOpen(outSerID)) {   // 如果该虚拟机迁空了的话
		//server.deleteVmTar(outSerID);  // 移除空的服务器
		updateEmpVmTarOrder(server, empVmTarOrder, outSerID);  // 加入空排序
		return true;   // 表示有空的服务器了
	}
	else {
		return false;   // 没有空的服务器
	}
}

// 迁移到非空服务器中
bool migToNoEmptySer(cSSP_Mig_Server &server, int &needCPUa, int &needCPUb, int &needRAMa, int &needRAMb, vector<double> &args,
	unordered_map<int, sMyEachServer> &delSerSet, int outSerID, unordered_set<int> &outSerIDSet,
	cyt::sServerItem &myServer, cyt::sServerItem &preServer, int &cntMig, cSSP_Mig_VM &VM, unordered_map<string, int> &dayWorkingVM,
	int whichDay, map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int &minEnergy, int &i) {

	// 从非空的服务器中找到合适的
	myServer = chooseNoEmptySer(server, needCPUa, needCPUb, needRAMa, needRAMb, args, delSerSet, outSerID, outSerIDSet);
	// myServer = get<0>(findMigInSer(server, needCPUa, needRAMa, needCPUb, needRAMb, delSerSet, true, false, outSerID, false));
	if (myServer.hardCost == -1) {    // 从非空中找到了合适的服务器
		preServer = myServer;   // 记录这个值
		outSerIDSet.insert(outSerID);   // 将这个可以迁出的服务器记录一下
		cntMig += server.serverVMSet[outSerID].size();   // 更新迁移数量
	}
	else {
		if (outSerIDSet.size() >= 1) {   // 表示在这之前已经记录了可以迁移的非空服务器
			for (auto &outSerID : outSerIDSet) {   // 找出所有这些可以迁移的服务器并迁移
				migrateSer(server, VM, outSerID, dayWorkingVM, empVmTarOrder, whichDay, delSerSet,
					preServer.buyID, args);
			}
			needCPUa = 0; needCPUb = 0; needRAMa = 0; needRAMb = 0;  // 清空数据
			minEnergy = 0;
			outSerIDSet.clear();
			i -= 2;
		}
		else
			return false;   // 表示在非空里已经无法放入虚拟机了
	}

	return true;

}

// 迁移到空的服务器中
bool migToEmptySer(cSSP_Mig_Server &server, int &needCPUa, int &needCPUb, int &needRAMa, int &needRAMb, vector<double> &args,
	unordered_map<int, sMyEachServer> &delSerSet, int outSerID, unordered_set<int> &outSerIDSet,
	cyt::sServerItem &myServer, cyt::sServerItem &preServer, int &cntMig, cSSP_Mig_VM &VM, unordered_map<string, int> &dayWorkingVM,
	int whichDay, map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int &minEnergy, int &i, bool &findNoEmpty) {

	// 从空的服务器中找到合适的
	myServer = chooseEmptySer(server, needCPUa, needCPUb, needRAMa, needRAMb, args, delSerSet,  // 从空的服务器中找合适的
		minEnergy, empVmTarOrder);
	if (myServer.hardCost == -1) {   // 表示从空的服务器中找到了合适的服务器
		preServer = myServer;   // 记录该值
		outSerIDSet.insert(outSerID);
		cntMig += server.serverVMSet[outSerID].size();   // 更新迁移数量
	}
	else {   // 到这里说明空的服务器也找不到合适的服务器了，先迁移
		if (outSerIDSet.size() >= 1) {   // 表示能找到空的服务器，则继续
			for (auto &outSerID : outSerIDSet) {   // 找出所有这些可以迁移的服务器并迁移
				migrateSer(server, VM, outSerID, dayWorkingVM, empVmTarOrder, whichDay, delSerSet,
					preServer.buyID, args);
				deleteEmpVmTar(server, empVmTarOrder, preServer.buyID);  // 加入的这个空服务器需要从空排序中拿走
			}
			needCPUa = 0; needCPUb = 0; needRAMa = 0; needRAMb = 0;  // 清空数据
			minEnergy = 0;
			outSerIDSet.clear();
			findNoEmpty = true;
			i -= 2;  //   这里暂时先这样，可能会让速度变慢/////////////????????????????????????????????
		}
		else
			return false;  // 到这里说明空的也找不全了
	}

	return true;

}


// 选择第一次选到的服务器
cyt::sServerItem chooseFirstServer(cSSP_Mig_Server &server, sVmItem &requestVM, cSSP_Mig_VM &VM, int index,
	unordered_map<int, sMyEachServer> &delSerSet, unordered_set<int> &emptySer, vector<double> &args,
	string vmID) {

	cyt::sServerItem myServer;
	myServer.hardCost = 1;   // hardCost = 1表示没有找到合适的服务器

	if (server.myServerSet.size() > 0) {    // 有服务器才开始找

		sMyEachServer tempServer;
		int minValue = INT_MAX;   // 记录最小值
		int needCPUa, needRAMa, needCPUb, needRAMb;

		if (requestVM.nodeStatus) {   // 双节点 

			needCPUa = requestVM.needCPU / 2;
			needRAMa = requestVM.needRAM / 2;
			needCPUb = requestVM.needCPU / 2;
			needRAMb = requestVM.needRAM / 2;

			/*		int outSerID = server.vmSourceOrder[index].first;
					tempServer = server.myServerSet[outSerID];
					int restCPU = tempServer.aIdleCPU + tempServer.bIdleCPU;
					int restRAM = tempServer.aIdleRAM + tempServer.bIdleRAM;
					minValue = restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3];*/

			for (auto itcpua = greaterEqu1(server.vmTarOrder, needCPUa); itcpua != server.vmTarOrder.end(); itcpua++) {
				for (auto itrama = greaterEqu2(itcpua->second, needRAMa); itrama != itcpua->second.end(); itrama++) {
					for (auto itcpub = greaterEqu3(itrama->second, needCPUb); itcpub != itrama->second.end(); itcpub++) {
						for (auto itramb = greaterEqu4(itcpub->second, needRAMb); itramb != itcpub->second.end(); itramb++) {
							for (int inSerID : itramb->second) {

								if (emptySer.count(inSerID) == 1) { continue; }
								if (inSerID == server.vmSourceOrder[index].first) {
									continue;
								}

								sMyEachServer tempServer;
								if (delSerSet.count(inSerID) == 1) {   // 该服务器在当天有删除操作
									tempServer = delSerSet[inSerID];
								}
								else {  // 表示这台服务器当天没有删除操作
									tempServer = server.myServerSet[inSerID];  // 既然要根据虚拟机来，排序就没有用了，遍历所有服务器
								}

								if (tempServer.aIdleCPU < needCPUa || tempServer.bIdleCPU < needCPUb ||
									tempServer.aIdleRAM < needRAMa || tempServer.bIdleRAM < needRAMb) {
									break;
								}

								tempServer = server.myServerSet[inSerID];   // 最后算比较值的时候还是得用myServerSet里的值
								int restCPU = tempServer.aIdleCPU + tempServer.bIdleCPU - requestVM.needCPU;
								int restRAM = tempServer.aIdleRAM + tempServer.bIdleRAM - requestVM.needRAM;
								int tempValue = int(restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3]);
								if (tempValue < minValue) {
									minValue = tempValue;
									myServer.energyCost = -1;
									myServer.hardCost = -1;
									myServer.buyID = inSerID;   // 记录服务器
								}
							}
						}
					}
				}
			}

			if (myServer.hardCost == -1) {
				int outSerID = server.vmSourceOrder[index].first;
				tempServer = server.myServerSet[outSerID];
				int restCPU = tempServer.aIdleCPU + tempServer.bIdleCPU;
				int restRAM = tempServer.aIdleRAM + tempServer.bIdleRAM;
				int tempValue = int(restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3]);
				if (tempValue < minValue) {
					myServer.hardCost = 1;
				}
			}

		}
		else {  // 单节点
			sEachWorkingVM workVm = VM.workingVmSet[vmID];
			/*node a*/
			{
				needCPUa = requestVM.needCPU;
				needRAMa = requestVM.needRAM;
				needCPUb = 0;
				needRAMb = 0;

				//int outSerID = server.vmSourceOrder[index].first;
				//tempServer = server.myServerSet[outSerID];
				//int restCPU = tempServer.aIdleCPU;
				//int restRAM = tempServer.aIdleRAM;
				//minValue = restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3];


				for (auto itcpua = greaterEqu1(server.vmTarOrder, needCPUa); itcpua != server.vmTarOrder.end(); itcpua++) {
					for (auto itrama = greaterEqu2(itcpua->second, needRAMa); itrama != itcpua->second.end(); itrama++) {
						for (auto itcpub = greaterEqu3(itrama->second, needCPUb); itcpub != itrama->second.end(); itcpub++) {
							for (auto itramb = greaterEqu4(itcpub->second, needRAMb); itramb != itcpub->second.end(); itramb++) {
								for (int inSerID : itramb->second) {

									if (emptySer.count(inSerID) == 1) { continue; }
									if (inSerID == server.vmSourceOrder[index].first && workVm.node == true) {
										continue;
									}

									sMyEachServer tempServer;
									if (delSerSet.count(inSerID) == 1) {  // 该服务器在当天有删除操作
										tempServer = delSerSet[inSerID];
									}
									else {  // 表示这台服务器当天没有删除操作
										tempServer = server.myServerSet[inSerID];  // 既然要根据虚拟机来，排序就没有用了，遍历所有服务器
									}

									if (tempServer.aIdleCPU < needCPUa || tempServer.aIdleRAM < needRAMa) {
										break;
									}

									tempServer = server.myServerSet[inSerID];   // 最后算比较值的时候还是得用myServerSet里的值
									int restCPU, restRAM;
									restCPU = tempServer.aIdleCPU - requestVM.needCPU;
									restRAM = tempServer.aIdleRAM - requestVM.needRAM;

									int tempValue = int(restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3]);
									if (tempValue < minValue) {
										minValue = tempValue;
										myServer.energyCost = -1;
										myServer.hardCost = -1;
										myServer.buyID = inSerID;   // 记录该服务器
										myServer.node = true;   // 返回true表示a 节点
									}
								}
							}
						}
					}
				}

				if (myServer.hardCost == -1) {
					int outSerID = server.vmSourceOrder[index].first;
					tempServer = server.myServerSet[outSerID];
					int restCPU = tempServer.aIdleCPU;
					int restRAM = tempServer.aIdleRAM;
					int tempValue = int(restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3]);
					if (tempValue < minValue) {
						myServer.hardCost = 1;
					}
				}

			}
			/*node b*/
			{
				needCPUa = 0;
				needRAMa = 0;
				needCPUb = requestVM.needCPU;
				needRAMb = requestVM.needRAM;

				/*int outSerID = server.vmSourceOrder[index].first;
				tempServer = server.myServerSet[outSerID];
				int restCPU = tempServer.bIdleCPU;
				int restRAM = tempServer.bIdleRAM;
				minValue = restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3];*/

				for (auto itcpua = greaterEqu1(server.vmTarOrder, needCPUa); itcpua != server.vmTarOrder.end(); itcpua++) {
					for (auto itrama = greaterEqu2(itcpua->second, needRAMa); itrama != itcpua->second.end(); itrama++) {
						for (auto itcpub = greaterEqu3(itrama->second, needCPUb); itcpub != itrama->second.end(); itcpub++) {
							for (auto itramb = greaterEqu4(itcpub->second, needRAMb); itramb != itcpub->second.end(); itramb++) {
								for (int inSerID : itramb->second) {

									if (emptySer.count(inSerID) == 1) { continue; }
									if (inSerID == server.vmSourceOrder[index].first && workVm.node == false) {
										continue;
									}

									sMyEachServer tempServer;
									if (delSerSet.count(inSerID) == 1) {  // 该服务器在当天有删除操作
										tempServer = delSerSet[inSerID];
									}
									else {  // 表示这台服务器当天没有删除操作
										tempServer = server.myServerSet[inSerID];  // 既然要根据虚拟机来，排序就没有用了，遍历所有服务器
									}

									if (tempServer.bIdleCPU < needCPUb || tempServer.bIdleRAM < needRAMb) {
										break;
									}

									tempServer = server.myServerSet[inSerID];   // 最后算比较值的时候还是得用myServerSet里的值

									int restCPU, restRAM;
									restCPU = tempServer.bIdleCPU - requestVM.needCPU;
									restRAM = tempServer.bIdleRAM - requestVM.needRAM;

									int tempValue = int(restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3]);
									if (tempValue < minValue) {
										minValue = tempValue;
										myServer.energyCost = -1;
										myServer.hardCost = -1;
										myServer.buyID = inSerID;   // 记录该服务器
										myServer.node = false;   // 返回true表示a 节点
									}
								}
							}
						}
					}
				}
			}

			if (myServer.hardCost == -1) {
				int outSerID = server.vmSourceOrder[index].first;
				tempServer = server.myServerSet[outSerID];
				int restCPU = tempServer.bIdleCPU;
				int restRAM = tempServer.bIdleRAM;
				int tempValue = int(restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3]);
				if (tempValue < minValue) {
					myServer.hardCost = 1;
				}
			}
		}
	}
	return myServer;
}

// 从空的服务器里找能耗最低的
cyt::sServerItem chooseEmptySer(cSSP_Mig_Server &server, int needCPUa, int needCPUb, int needRAMa, int needRAMb,
	vector<double> &args, unordered_map<int, sMyEachServer> &delSerSet, int minEnergy,
	map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder) {

	cyt::sServerItem ompServer[2];
	ompServer[0].hardCost = 1;
	ompServer[1].hardCost = 1;
	int ompMinValue[2];
	ompMinValue[0] = minEnergy;
	ompMinValue[1] = minEnergy;
	vector<map<int, map<int, map<int, map<int, vector<int>>>>>::iterator> ites; 
	for (auto itcpua = greaterEqu1(empVmTarOrder, needCPUa); itcpua != empVmTarOrder.end(); itcpua++) {
		ites.push_back(itcpua);
	}
	#pragma omp parallel for num_threads(2)
	for (int i = 0; i < (int)ites.size(); i++) {
		auto itcpua = ites[i];
		for (auto itrama = greaterEqu2(itcpua->second, needRAMa); itrama != itcpua->second.end(); itrama++) {
			for (auto itcpub = greaterEqu3(itrama->second, needCPUb); itcpub != itrama->second.end(); itcpub++) {
				for (auto itramb = greaterEqu4(itcpub->second, needRAMb); itramb != itcpub->second.end(); itramb++) {
					for (int inSerID : itramb->second) {

						// 空的就不会往自己身上迁移了,所以不用进行ID检测

						// 当天要删除操作，需要查看是否符合删除前的限制
						sMyEachServer tempServer;
						if (delSerSet.count(inSerID) == 1) {
							tempServer = delSerSet[inSerID];
							if (tempServer.aIdleCPU < needCPUa || tempServer.bIdleCPU < needCPUb ||
								tempServer.aIdleRAM < needRAMa || tempServer.bIdleRAM < needRAMb)
								continue;   // 不符合删除前的条件则继续
						}
						tempServer = server.myServerSet[inSerID];
						sServerItem serInfo;
						serInfo = server.info[tempServer.serName];

						if (serInfo.energyCost < ompMinValue[omp_get_thread_num()]) {  // 选择能耗最低的服务器作为迁入
							ompMinValue[omp_get_thread_num()] = serInfo.energyCost;
							ompServer[omp_get_thread_num()].energyCost = -1;
							ompServer[omp_get_thread_num()].hardCost = -1;
							ompServer[omp_get_thread_num()].buyID = inSerID;
						}

					}
				}
			}
		}
	}
	/*线程合并结果*/
	cyt::sServerItem myServer;
	int betValue;
	if (ompMinValue[0] <= ompMinValue[1]) {
		myServer = ompServer[0];
		betValue = ompMinValue[0];
	}
	else {
		myServer = ompServer[1];
		betValue = ompMinValue[1];
	}


	return myServer;
}

// 从非空的服务器中选择合适的
cyt::sServerItem chooseNoEmptySer(cSSP_Mig_Server &server, int needCPUa, int needCPUb, int needRAMa, int needRAMb,
	vector<double> &args, unordered_map<int, sMyEachServer> &delSerSet, int outSerID,
	unordered_set<int> outSerIDSet) {

	cyt::sServerItem ompServer[2];
	ompServer[0].hardCost = 1;
	ompServer[1].hardCost = 1;
	int ompMinValue[2];
	ompMinValue[0] = INT_MAX;
	ompMinValue[1] = INT_MAX;
	vector<map<int, map<int, map<int, map<int, vector<int>>>>>::iterator> ites; 
	for (auto itcpua = greaterEqu1(server.vmTarOrder, needCPUa); itcpua != server.vmTarOrder.end(); itcpua++) {
		ites.push_back(itcpua);
	}
	#pragma omp parallel for num_threads(2)
	for (int i = 0; i < (int)ites.size(); i++) {
		auto itcpua = ites[i];
		for (auto itrama = greaterEqu2(itcpua->second, needRAMa); itrama != itcpua->second.end(); itrama++) {
			for (auto itcpub = greaterEqu3(itrama->second, needCPUb); itcpub != itrama->second.end(); itcpub++) {
				for (auto itramb = greaterEqu4(itcpub->second, needRAMb); itramb != itcpub->second.end(); itramb++) {
					for (int inSerID : itramb->second) {

						// 如果该服务器是自己的话，继续(暂时不考虑放在自己上)
						if (inSerID == outSerID || outSerIDSet.count(inSerID) == 1)
							continue;

						// 当天要删除操作，需要查看是否符合删除前的限制
						sMyEachServer tempServer;
						if (delSerSet.count(inSerID) == 1) {
							tempServer = delSerSet[inSerID];
							if (tempServer.aIdleCPU < needCPUa || tempServer.bIdleCPU < needCPUb ||
								tempServer.aIdleRAM < needRAMa || tempServer.bIdleRAM < needRAMb)
								continue;   // 不符合删除前的条件则继续
						}
						tempServer = server.myServerSet[inSerID];
						int restCPU = tempServer.aIdleCPU + tempServer.bIdleCPU - (needCPUa + needCPUb);
						int restRAM = tempServer.aIdleRAM + tempServer.bIdleRAM - (needRAMa + needRAMb);
						int tempValue = restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3];
						// int tempValue = getGoal(server, inSerID, true, false, needCPUa + needCPUb, needRAMa + needRAMb);

						if (tempValue < ompMinValue[omp_get_thread_num()]) {  // 选择最合适的服务器作为迁入对象
							ompMinValue[omp_get_thread_num()] = tempValue;
							ompServer[omp_get_thread_num()].energyCost = -1;
							ompServer[omp_get_thread_num()].hardCost = -1;
							ompServer[omp_get_thread_num()].buyID = inSerID;
						}
					}
				}
			}
		}
	}
	/*线程合并结果*/
	cyt::sServerItem myServer;
	myServer.hardCost = 1; 
	int minValue = INT_MAX; 
	if (ompMinValue[0] <= ompMinValue[1]) {
		myServer = ompServer[0];
		minValue = ompMinValue[0];
	}
	else {
		myServer = ompServer[1];
		minValue = ompMinValue[1];
	}

	return myServer;
}