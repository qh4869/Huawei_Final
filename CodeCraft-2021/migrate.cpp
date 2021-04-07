#include "migrate.h"

// 通过多次迭代来实现迁移
void migrateVM_3(cServer &server, cVM &VM, int whichDay, unordered_map<string, int> &dayWorkingVM,
	int vmTotalNum, unordered_map<int, sMyEachServer> &delSerSet, vector<double> &args) {

	int migrateNum = vmTotalNum * 30 / 1000;   // 迁移数量
	if (migrateNum <= 0) { return; }
	int cntMig = 0;   // 迁移的服务器数量统计	
	unordered_set<int> emptySer;    // 空的服务器集合，该服务器不可作为迁入服务器
	double ratio = 2;
	//subMigrateVM(server, VM, whichDay, delSerSet, dayWorkingVM, emptySer, args, cntMig, migrateNum, ratio);
	//subMigrateVM(server, VM, whichDay, delSerSet, dayWorkingVM, emptySer, args, cntMig, migrateNum, ratio);
	massMigrate(server, VM, whichDay, dayWorkingVM, delSerSet, cntMig, migrateNum, args);

}

void migrateVM_4(cServer &server, cVM &VM, int whichDay, unordered_map<string, int> &dayWorkingVM,
	int vmTotalNum, unordered_map<int, sMyEachServer> &delSerSet, vector<double> &args, int &cntMig,
	int migrateNum, int loopNum) {

	int loopCnt = 0;
	while (loopCnt < loopNum) {
		massMigrate(server, VM, whichDay, dayWorkingVM, delSerSet, cntMig, migrateNum, args);
		loopCnt++;
	}

}

void subMigrateVM(cServer &server, cVM &VM, int whichDay, unordered_map<int, sMyEachServer> &delSerSet,
	unordered_map<string, int> &dayWorkingVM, unordered_set<int> &emptySer, vector<double> &args,
	int &cntMig, int &migrateNum, double ratio) {

	int serID;    // 迁出的serID
	string vmID;  // 服务器ID
	sEachWorkingVM workVM;   // 工作中的虚拟机
	sVmItem requestVM;   // 记录虚拟机需要的资源
	sServerItem myServer;
	unordered_map<string, int> tempSerVmSet;
	map<int, map<int, map<int, map<int, vector<int>>>>> empVmTarOrder;    // 用于存储空的服务器的顺序

	for (int i = 0; i < (int)server.vmSourceOrder.size() / ratio; i++) {   // 遍历一次服务器，把可以迁出的迁出

		serID = server.vmSourceOrder[i].first;   // 迁出的服务器id
		if (server.vmSourceOrder[i].second == 0) {
			server.deleteVmTar(serID);
			updateEmpVmTarOrder(server, empVmTarOrder, serID);
		}

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
			workVM = VM.workingVmSet[vmID];
			requestVM = VM.info[workVM.vmName];

			myServer = chooseFirstServer(server, requestVM, VM, i, delSerSet, emptySer, args, vmID);
			if (myServer.hardCost == -1) {   // 表示找到了合适的服务器
				cntMig++;
				if (requestVM.nodeStatus) {  // true : double node
					if (myServer.buyID == serID) {
						continue;
					}
					if (dayWorkingVM.count(vmID) == 1) {
						VM.reDeploy(server, whichDay, vmID, workVM.vmName, myServer.buyID, args);
						if (delSerSet.count(myServer.buyID) == 1) {
							updateDelSerSet(delSerSet, requestVM, myServer.node, myServer.buyID, true);  // 添加操作
						}
						else if (delSerSet.count(serID) == 1) {
							updateDelSerSet(delSerSet, requestVM, workVM.node, serID, false);   // 删除操作
						}
						continue;
					}

					VM.transfer(server, whichDay, vmID, myServer.buyID);
					server.updatVmTarOrder(requestVM.needCPU / 2, requestVM.needRAM / 2, requestVM.needCPU / 2, requestVM.needRAM / 2,
						serID, false);
					server.updatVmTarOrder(requestVM.needCPU / 2, requestVM.needRAM / 2, requestVM.needCPU / 2, requestVM.needRAM / 2,
						myServer.buyID, true);

				}
				else {   // false : single node
					if (myServer.buyID == serID && myServer.node == workVM.node) {
						continue;
					}
					if (dayWorkingVM.count(vmID) == 1) {
						VM.reDeploy(server, whichDay, vmID, workVM.vmName, myServer.buyID, myServer.node, args);
						if (delSerSet.count(myServer.buyID) == 1) {
							updateDelSerSet(delSerSet, requestVM, myServer.node, myServer.buyID, true);  // 添加操作
						}
						else if (delSerSet.count(serID) == 1) {
							updateDelSerSet(delSerSet, requestVM, workVM.node, serID, false);   // 删除操作
						}
						continue;
					}
					VM.transfer(server, whichDay, vmID, myServer.buyID, myServer.node);
					if (myServer.buyID == serID) {
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
							server.updatVmTarOrder(requestVM.needCPU, requestVM.needRAM, 0, 0, serID, false);
						else
							server.updatVmTarOrder(0, 0, requestVM.needCPU, requestVM.needRAM, serID, false);
					}
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

		if (!server.isOpen(serID)) {      // 空的服务器就不迁移了，直接继续
			server.deleteVmTar(serID);
			emptySer.insert(serID);
			continue;
		}

	}

}


// 选择第一次选到的服务器
sServerItem chooseFirstServer(cServer &server, sVmItem &requestVM, cVM &VM, int index,
	unordered_map<int, sMyEachServer> &delSerSet, unordered_set<int> &emptySer, vector<double> &args,
	string vmID) {

	sServerItem myServer;
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

			auto it = greaterEqu1(server.vmTarOrder, needCPUa);
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

//////////////////////////////////////主要的迁移函数/////////////////////////////////////////////////
// 一次迁移大量的虚拟机
void massMigrate(cServer &server, cVM &VM, int whichDay, unordered_map<string, int> &dayWorkingVM,
	unordered_map<int, sMyEachServer> &delSerSet, int &cntMig, int &migrateNum, vector<double> &args) {

	map<int, map<int, map<int, map<int, vector<int>>>>> empVmTarOrder;   // 用于记录空服务器的排序
	unordered_set<int> outSerIDSet;    // 迁出的服务器ID集合
	int outSerID;   // 迁出的服务器ID
	int needCPUa = 0, needCPUb = 0, needRAMa = 0, needRAMb = 0, minEnergy = 0;  // 记录需要的资源以及能耗
	sServerItem myServer, preServer;  // 当前的最优迁入服务器以及上一次的最优服务器
	sMyEachServer outSer;   // 迁出的服务器
	sServerItem outSerInfo;  // 迁出的服务器原始信息
	bool findNoEmpty = true;  // true表示能在非空集合中找到
	bool findEmpty = true;  // true表示能在空服务器中找到
	bool buyNew = false;  // true表示可以购买新的服务器
	int empConut = 0, maxCtrl = 50;
	int maxBuyNum = 0, buyCount = 0, Cost = 0;
	int maxBreakCount = INT_MAX, breakCnt = 0;
	if (whichDay > 100 && whichDay < 110)
		maxBuyNum = 3;
	else
		buyNew = false;

	for (int i = 0; i < server.vmSourceOrder.size(); i++) {

		outSerID = server.vmSourceOrder[i].first;  // 迁出的服务器ID
		outSer = server.myServerSet[outSerID];   // 迁出的服务器

		/////////////////////////////////////////////////////////////////////////////////////////////
		//////////// sServerItem结构体增加了buyID,node以及serName字段 ////////////////////////////
		outSerInfo = server.info[outSer.serName];  // 迁出的服务器的原始信息
		/////////////////////////////////////////////////////////////////////////////////////////////

		if ((double)server.vmSourceOrder[i].second / (double)(outSerInfo.totalCPU + outSerInfo.totalRAM) > 0.97) {
			continue;
		}

		// 没有可以迁出的资源则继续
		if (server.vmSourceOrder[i].second == 0) {
			/////////////////////////////下面这三个函数都是新加的/////////////////////////
			server.deleteVmTar(outSerID);   // 删除空的服务器
			deleteEmpVmTar(server, empVmTarOrder, outSerID);  // 避免重复加入，先删除
			updateEmpVmTarOrder(server, empVmTarOrder, outSerID);  // 加入空的排序
			continue;
		}

		if (cntMig + server.serverVMSet[outSerID].size() > migrateNum) {   // 达到最大容量，不能再继续下去了
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
		if (!findEmpty && !buyNew) {   // 表示不能从空的地方找到符合的服务器
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
		if (buyNew) {
			if (buyNewSer(server, needCPUa, needCPUb, needRAMa, needRAMb, outSerIDSet,
				Cost, preServer, outSerID, myServer, VM, dayWorkingVM, whichDay, args,
				empVmTarOrder, delSerSet, i, cntMig, migrateNum)) {
				buyCount++;
				if (buyCount >= maxBuyNum)
					buyNew = false;
				findEmpty = true;
				findNoEmpty = true;
				minEnergy = 0;
			}
		}

	}

}


// 从非空的服务器中选择合适的
sServerItem chooseNoEmptySer(cServer &server, int needCPUa, int needCPUb, int needRAMa, int needRAMb,
	vector<double> &args, unordered_map<int, sMyEachServer> &delSerSet, int outSerID,
	unordered_set<int> outSerIDSet) {

	sServerItem myServer;
	sMyEachServer tempServer;  // 拥有记录该服务器的真实信息
	myServer.hardCost = 1;   // hardCost = 1 表示没有找到合适的服务器
	int minValue = INT_MAX; // 记录找到的最小值
	int restCPU, restRAM, tempValue;  // 用于记录剩余资源以及临时的最小值

	auto it = greaterEqu1(server.vmTarOrder, needCPUa);
	for (auto itcpua = greaterEqu1(server.vmTarOrder, needCPUa); itcpua != server.vmTarOrder.end(); itcpua++) {
		for (auto itrama = greaterEqu2(itcpua->second, needRAMa); itrama != itcpua->second.end(); itrama++) {
			for (auto itcpub = greaterEqu3(itrama->second, needCPUb); itcpub != itrama->second.end(); itcpub++) {
				for (auto itramb = greaterEqu4(itcpub->second, needRAMb); itramb != itcpub->second.end(); itramb++) {
					for (int inSerID : itramb->second) {

						// 如果该服务器是自己的话，继续(暂时不考虑放在自己上)
						if (inSerID == outSerID || outSerIDSet.count(inSerID) == 1)
							continue;

						// 当天要删除操作，需要查看是否符合删除前的限制
						if (delSerSet.count(inSerID) == 1) {
							tempServer = delSerSet[inSerID];
							if (tempServer.aIdleCPU < needCPUa || tempServer.bIdleCPU < needCPUb ||
								tempServer.aIdleRAM < needRAMa || tempServer.bIdleRAM < needRAMb)
								continue;   // 不符合删除前的条件则继续
						}
						tempServer = server.myServerSet[inSerID];
						restCPU = tempServer.aIdleCPU + tempServer.bIdleCPU - (needCPUa + needCPUb);
						restRAM = tempServer.aIdleRAM + tempServer.bIdleRAM - (needRAMa + needRAMb);
						tempValue = restCPU + restRAM + abs(restCPU - args[2] * restRAM) * args[3];

						if (tempValue < minValue) {  // 选择最合适的服务器作为迁入对象
							minValue = tempValue;
							myServer.energyCost = -1;
							myServer.hardCost = -1;
							myServer.buyID = inSerID;
						}

					}
				}
			}
		}
	}

	return myServer;

}

// 将一个服务器里的资源集体迁移
void migrateSer(cServer &server, cVM &VM, int outSerID, unordered_map<string, int> &dayWorkingVM,
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
		server.updatVmSourceOrder(requestVM, outSerID, false, args);      // 删除操作
		server.updatVmSourceOrder(requestVM, inSerID, true, args);   // 添加操作
	}

	server.deleteVmTar(outSerID);  // 虚拟机迁移结束之后，该服务器就空了
	deleteEmpVmTar(server, empVmTarOrder, outSerID);
	updateEmpVmTarOrder(server, empVmTarOrder, outSerID);   // 将该空的虚拟机加入空的排序里

}


// 从空的服务器里找能耗最低的
sServerItem chooseEmptySer(cServer &server, int needCPUa, int needCPUb, int needRAMa, int needRAMb,
	vector<double> &args, unordered_map<int, sMyEachServer> &delSerSet, int minEnergy,
	map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder) {

	sServerItem myServer, serInfo;
	sMyEachServer tempServer;  // 拥有记录该服务器的真实信息
	myServer.hardCost = 1;   // hardCost = 1 表示没有找到合适的服务器

	auto it = greaterEqu1(empVmTarOrder, needCPUa);
	for (auto itcpua = greaterEqu1(empVmTarOrder, needCPUa); itcpua != empVmTarOrder.end(); itcpua++) {
		for (auto itrama = greaterEqu2(itcpua->second, needRAMa); itrama != itcpua->second.end(); itrama++) {
			for (auto itcpub = greaterEqu3(itrama->second, needCPUb); itcpub != itrama->second.end(); itcpub++) {
				for (auto itramb = greaterEqu4(itcpub->second, needRAMb); itramb != itcpub->second.end(); itramb++) {
					for (int inSerID : itramb->second) {

						// 空的就不会往自己身上迁移了,所以不用进行ID检测

						// 当天要删除操作，需要查看是否符合删除前的限制
						if (delSerSet.count(inSerID) == 1) {
							tempServer = delSerSet[inSerID];
							if (tempServer.aIdleCPU < needCPUa || tempServer.bIdleCPU < needCPUb ||
								tempServer.aIdleRAM < needRAMa || tempServer.bIdleRAM < needRAMb)
								continue;   // 不符合删除前的条件则继续
						}
						tempServer = server.myServerSet[inSerID];
						serInfo = server.info[tempServer.serName];

						if (serInfo.energyCost < minEnergy) {  // 选择能耗最低的服务器作为迁入
							minEnergy = serInfo.energyCost;
							myServer.energyCost = -1;
							myServer.hardCost = -1;
							myServer.buyID = inSerID;
						}

					}
				}
			}
		}
	}

	return myServer;

}


// 将服务器里的虚拟机一个个尝试性的往外迁移
bool migrateSerVm(cServer &server, cVM &VM, int outSerID, unordered_map<string, int> &dayWorkingVM,
	map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int whichDay, int index,
	unordered_map<int, sMyEachServer> &delSerSet, vector<double> &args, int &cntMig, int migrateNum) {

	unordered_map<string, int> tempSerVmSet = server.serverVMSet[outSerID];  // 需要尝试迁出的所有虚拟机集合
	string vmID;
	sEachWorkingVM workVM;  // 工作中的虚拟机
	sVmItem requestVM;   // 记录虚拟机需要的资源
	sServerItem myServer;
	unordered_set<int> emptySer;

	for (auto ite = tempSerVmSet.begin(); ite != tempSerVmSet.end(); ite++) {
		if (cntMig >= migrateNum) // 达到数量限制，直接退出
			return false;
		vmID = ite->first;   // 取出虚拟机ID
		workVM = VM.workingVmSet[vmID];   // 取出工作中的虚拟机
		requestVM = VM.info[workVM.vmName];   // 取出该虚拟机的资源信息
		myServer = chooseFirstServer(server, requestVM, VM, index, delSerSet, emptySer, args, vmID);
		if (myServer.hardCost == -1) {   // 表示找到了合适的服务器
			cntMig++;
			if (requestVM.nodeStatus) {  // true : double node
				if (myServer.buyID == outSerID) {
					continue;
				}
				if (dayWorkingVM.count(vmID) == 1) {
					VM.reDeploy(server, whichDay, vmID, workVM.vmName, myServer.buyID, args);
					if (delSerSet.count(myServer.buyID) == 1) {
						updateDelSerSet(delSerSet, requestVM, myServer.node, myServer.buyID, true);  // 添加操作
					}
					else if (delSerSet.count(outSerID) == 1) {
						updateDelSerSet(delSerSet, requestVM, workVM.node, outSerID, false);   // 删除操作
					}
					continue;
				}

				VM.transfer(server, whichDay, vmID, myServer.buyID);
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
					if (delSerSet.count(myServer.buyID) == 1) {
						updateDelSerSet(delSerSet, requestVM, myServer.node, myServer.buyID, true);  // 添加操作
					}
					else if (delSerSet.count(outSerID) == 1) {
						updateDelSerSet(delSerSet, requestVM, workVM.node, outSerID, false);   // 删除操作
					}
					continue;
				}
				VM.transfer(server, whichDay, vmID, myServer.buyID, myServer.node);
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
			server.updatVmSourceOrder(requestVM, outSerID, false, args);      // 删除操作
			server.updatVmSourceOrder(requestVM, myServer.buyID, true, args);   // 添加操作

		}
		else {   // 没有找到合适的服务器
			break;
		}
	}

	if (!server.isOpen(outSerID)) {   // 如果该虚拟机迁空了的话
		server.deleteVmTar(outSerID);  // 移除空的服务器
		updateEmpVmTarOrder(server, empVmTarOrder, outSerID);  // 加入空排序
		return true;   // 表示有空的服务器了
	}
	else {
		return false;   // 没有空的服务器
	}
}

// 实在是空不了了，可以考虑购买新的服务器
bool buyNewSer(cServer &server, int &needCPUa, int &needCPUb, int &needRAMa, int &needRAMb,
	unordered_set<int> &outSerIDSet, int &Cost, sServerItem &preServer, int outSerID, sServerItem &myServer,
	cVM &VM, unordered_map<string, int> &dayWorkingVM, int whichDay, vector<double> &args,
	map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder,
	unordered_map<int, sMyEachServer> &delSerSet, int &i, int &cntMig, int migrateNum) {

	sServerItem outSerInfo;
	sMyEachServer outSer;
	myServer = chooseNewSer(server, needCPUa, needCPUb, needRAMa, needRAMb, Cost);
	while (myServer.serName != "") {    // 表示找到了新的服务器
		if (cntMig + server.serverVMSet[outSerID].size() > migrateNum)   // 超过容量限制则退出
			break;
		cntMig += server.serverVMSet[outSerID].size();
		outSerIDSet.insert(outSerID);   // 将该服务器加入
		preServer = myServer;
		i++;
		outSerID = server.vmSourceOrder[i].first;  // 迁出的服务器ID
		outSer = server.myServerSet[outSerID];   // 迁出的服务器
		outSerInfo = server.info[outSer.serName];  // 迁出的服务器的原始信息
		needCPUa += (outSerInfo.totalCPU / 2 - outSer.aIdleCPU);   // 需要迁出的资源数量
		needCPUb += (outSerInfo.totalCPU / 2 - outSer.bIdleCPU);
		needRAMa += (outSerInfo.totalRAM / 2 - outSer.aIdleRAM);
		needRAMb += (outSerInfo.totalRAM / 2 - outSer.bIdleRAM);
		myServer = chooseNewSer(server, needCPUa, needCPUb, needRAMa, needRAMb, Cost);
	}
	if (outSerIDSet.size() >= 2) {
		server.cyt_purchase(preServer.serName, whichDay);   // 购买新的服务器
		int serID = server.myServerSet.size() - 1;   // 新买的服务器的ID
		for (auto &outSerID : outSerIDSet) {   // 找出所有这些可以迁移的服务器并迁移
			migrateSer(server, VM, outSerID, dayWorkingVM, empVmTarOrder, whichDay, delSerSet,
				serID, args);   // 注意这里新买的服务器的ID
		}
		outSerIDSet.clear();
		needCPUa = 0; needCPUb = 0; needRAMa = 0; needRAMb = 0;
		return true;   // 表示此次为有效的购买
	}
	return false;   // 表示此次没有买到服务器

}

// 找到能满足条件的服务器，并且能耗和硬件成本最低
sServerItem chooseNewSer(cServer &server, int needCPUa, int needCPUb, int needRAMa,
	int needRAMb, int Cost) {

	sServerItem myServer, tempServer;
	myServer.serName = "";   // 没有serName表示没有找到合适的
	int serCost;

	for (int i = 0; i < server.priceOrder.size() / 10; i++) {
		//serCost = server.priceOrder[i].second;   // 购买该服务器的能耗和硬件成本之和
		//if (serCost > Cost)    // 购买新的成本过高，退出
		//	return myServer;
		tempServer = server.info[server.priceOrder[i].first];  // 根据名称取出服务器
		if (tempServer.totalCPU / 2 >= needCPUa && tempServer.totalCPU / 2 >= needCPUb &&
			tempServer.totalRAM / 2 >= needRAMa && tempServer.totalRAM / 2 >= needRAMb) {
			myServer = tempServer;    // 能走到这里说明serCost < Cost
			return myServer;
		}
	}

	return myServer;   // 走到这里说明找不到合适的服务器
}

// 迁移到非空服务器中
bool migToNoEmptySer(cServer &server, int &needCPUa, int &needCPUb, int &needRAMa, int &needRAMb, vector<double> &args,
	unordered_map<int, sMyEachServer> &delSerSet, int outSerID, unordered_set<int> &outSerIDSet,
	sServerItem &myServer, sServerItem &preServer, int &cntMig, cVM &VM, unordered_map<string, int> &dayWorkingVM,
	int whichDay, map<int, map<int, map<int, map<int, vector<int>>>>> &empVmTarOrder, int &minEnergy, int &i) {

	// 从非空的服务器中找到合适的
	myServer = chooseNoEmptySer(server, needCPUa, needCPUb, needRAMa, needRAMb, args, delSerSet, outSerID, outSerIDSet);
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
bool migToEmptySer(cServer &server, int &needCPUa, int &needCPUb, int &needRAMa, int &needRAMb, vector<double> &args,
	unordered_map<int, sMyEachServer> &delSerSet, int outSerID, unordered_set<int> &outSerIDSet,
	sServerItem &myServer, sServerItem &preServer, int &cntMig, cVM &VM, unordered_map<string, int> &dayWorkingVM,
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