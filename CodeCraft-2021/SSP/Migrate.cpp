#include "ssp.h"

double weightChart(int vmNum) {
	unordered_map<int, double> chart({{1, 2}, {2, 1.5}, {3, 1}, {4, 0.5}, {5, 0.3}, {-1, 0.1}});
	if (chart.count(vmNum))
		return chart.at(vmNum);
	else
		return chart.at(-1);
}

void dailyMigrate(int vmNumStart, unordered_map<int, sMyEachServer> &delSerSet,
	unordered_set<string> &dayWorkingVM, int iDay, cSSP_Mig_Server &server, cSSP_Mig_VM &VM, cSSP_Mig_Request &request) {

	/*先统计我可以迁移多少台*/
	int maxMigrateNum = vmNumStart * 3 / 100;
	// VM.migFind = maxMigrateNum * 15;
	int cntMig = 0; // 当天已经迁移了多少台
	VM.saveGoal.clear();

	/*自适应解vm锁*/
	request.getReqNumEachDay(iDay);
	if (request.delNum[iDay] > request.delLarge) { // 某一天的del特别多
		VM.stopSetCnt.clear();
		VM.delCnt = 0;
#ifdef LOCAL
		cout << "---------------" << iDay << ":全部清空" << endl;
#endif
	}
	else if (VM.delCnt > VM.delCntMax) { // del累计很多
		int sizeBeg = VM.stopSetCnt.size();
		for (auto it = VM.stopSetCnt.begin(); it != VM.stopSetCnt.end();) {
			if (it->second >= VM.delayTimes)
				it = VM.stopSetCnt.erase(it);
			else
				it++;
		}
		VM.delCnt = 0;
#ifdef LOCAL
		cout << "---------------" << iDay << ":部分清空:" << sizeBeg << "->" << VM.stopSetCnt.size() <<endl;
#endif
	}

	/*vmSourceOrder统计非空的个数*/
	int startI;
	for (int i = 0; i < (int)server.vmSourceOrder.size(); i++) {
		if (server.vmSourceOrder[i].second != 0) {
			startI = i;
			break;
		}
	}
	int endI = ( (int)server.vmSourceOrder.size() - startI ) / VM.ratio + startI;

	int cntIter = 0;
	int cntVM = 0;
	while (cntMig < maxMigrateNum && cntIter < VM.maxIter) {
		cntIter ++;

		for (int i = startI; i < endI; i++) { // i--服务器
			int outSerID = server.vmSourceOrder[i].first; // 迁出的服务器id

			unordered_map<string, int> tempSerVmSet = server.serverVMSet[outSerID];
			for (auto ite = tempSerVmSet.begin(); ite != tempSerVmSet.end(); ite++) { // 虚拟机
				if (cntVM++ >= VM.migFind) {
					//cout << iDay << ":" << "vm迁出" << endl;
					return;
				}

				if (cntMig == maxMigrateNum) {
					//cout << iDay << ":" << "迁移次数" << endl;
					return;
				}

				string vmID = ite->first; // 虚拟机id
				int outNodeTmp = ite->second; // 迁出服务器的节点
				bool outNode;
				if (outNodeTmp == 0) // node a
					outNode = true;
				else if (outNodeTmp == 1) // node b
					outNode = false;
				else // double
					outNode = false;

				if (dayWorkingVM.count(vmID) == 1) { continue; } // 暂时不迁移当天部署的虚拟机（可以等效于部署上)

				sVmItem requestVM = VM.info[VM.workingVmSet[vmID].vmName];
				bool vmIsDouble = requestVM.nodeStatus;
				int inSerID; // 迁入服务器id
				bool inNode = false; // 迁入服务器节点

				if (vmIsDouble)
					inSerID = srchInVmSourceDouble(server, requestVM, VM, i, delSerSet, vmID);
				else
					tie(inSerID, inNode) = srchInVmSourceSingle(server, requestVM, VM, i, delSerSet, vmID);

				if (inSerID != -1) { // 可以找到

					if (vmIsDouble) {   // true表示双节点
						if (inSerID == outSerID)
							continue; // 不能原地迁移

						VM.transfer(server, iDay, vmID, inSerID);
						VM.saveGoal.clear();

						/*更新vmTarOrder*/
						server.updatVmTarOrder(requestVM.needCPU / 2, requestVM.needRAM / 2, requestVM.needCPU / 2, requestVM.needRAM / 2,
							outSerID, false);
						server.updatVmTarOrder(requestVM.needCPU / 2, requestVM.needRAM / 2, requestVM.needCPU / 2, requestVM.needRAM / 2,
							inSerID, true);
					}
					else {
						if (inSerID == outSerID && inNode == outNode)
							continue; // 不能原地迁移

						VM.transfer(server, iDay, vmID, inSerID, inNode);
						VM.saveGoal.clear();

						/*更新vmTarOrder*/
						if (inSerID == outSerID) { // 同一台服务器
							if (outNode == false && inNode == true) { // node b -> node a
								server.updatVmTarOrder(requestVM.needCPU, requestVM.needRAM, -requestVM.needCPU,
									-requestVM.needRAM, inSerID, true);
							}
							else { // node a -> node b
								server.updatVmTarOrder(-requestVM.needCPU, -requestVM.needRAM, requestVM.needCPU,
									requestVM.needRAM, inSerID, true);
							}
						}
						else {
							if (inNode)  // node a
								server.updatVmTarOrder(requestVM.needCPU, requestVM.needRAM, 0, 0, inSerID, true);
							else
								server.updatVmTarOrder(0, 0, requestVM.needCPU, requestVM.needRAM, inSerID, true);
							if (outNode) // node a
								server.updatVmTarOrder(requestVM.needCPU, requestVM.needRAM, 0, 0, outSerID, false);
							else
								server.updatVmTarOrder(0, 0, requestVM.needCPU, requestVM.needRAM, outSerID, false);
						}
					}
					cntMig++;

					if (delSerSet.count(inSerID) == 1)   // 迁入的服务器当天有删除操作
						updateDelSerSet(delSerSet, requestVM, inNode, inSerID, true);  // 添加虚拟机
					else if (delSerSet.count(outSerID) == 1)   // 迁出的服务器当天有删除操作
						updateDelSerSet(delSerSet, requestVM, outNode, outSerID, false);   // 删除虚拟机
				
					server.updatVmSourceOrder(requestVM.needCPU, requestVM.needRAM, outSerID, false);
					server.updatVmSourceOrder(requestVM.needCPU, requestVM.needRAM, inSerID, true);
				}
				else
					continue;
			}
		}
	}

	/*if (cntMig >= maxMigrateNum)
		cout << iDay << ":" << "迁移次数" << endl;
	else
		cout << iDay << ":" << "迭代次数" << endl;*/

	
}

int srchInVmSourceDouble(cSSP_Mig_Server &server, sVmItem &requestVM, cSSP_Mig_VM &VM, int index,
	unordered_map<int, sMyEachServer> &delSerSet, string vmID) {
	/* Fn: 从server.vmSourceOrder中找一台服务器来迁入第index个服务器的requestVM 双节点
	*		-1表示找不到
	*/
	cyt::sServerItem myServer = bestFitMigrate(server, requestVM, VM, index, delSerSet, vmID);

	if (myServer.hardCost == -1) {    // 找到了服务器
		return myServer.buyID;
	}

	return -1;
}

tuple<int, bool> srchInVmSourceSingle(cSSP_Mig_Server &server, sVmItem &requestVM, cSSP_Mig_VM &VM, int index,
	unordered_map<int, sMyEachServer> &delSerSet, string vmID) {
	/* Fn: 单节点
	*/
	cyt::sServerItem myServer = bestFitMigrate(server, requestVM, VM, index, delSerSet, vmID);

	if (myServer.hardCost == -1) {    // 找到了服务器
		return { myServer.buyID, myServer.node };
	}

	return { -1, false };
}

cyt::sServerItem bestFitMigrate(cSSP_Mig_Server &server, sVmItem &requestVM, cSSP_Mig_VM &VM, int index,
	unordered_map<int, sMyEachServer> &delSerSet, string vmID) {
/* Fn: 选择决定是否迁移 && 选择迁移服务器
*
* Note:
*	- 调用的函数大多写在_ssp.cpp中
*/

	cyt::sServerItem myServer;
	myServer.hardCost = 1;   // 可通过hardCost来判断是否找到了服务器
	int minValue = INT_MAX;

	int outSerID = server.vmSourceOrder[index].first;
	bool outNode = VM.workingVmSet[vmID].node;
	string vmName = VM.workingVmSet[vmID].vmName;

	/*判断vm锁*/
	if (VM.isLocked(vmID))
		return myServer;

	if (requestVM.nodeStatus) {// 双节点 

		/*快速判断，提高速度*/
		if (quickFindDouble(myServer, server, VM, vmName, vmID, outSerID))
			return myServer;

		/*摘除自己的初值*/
		minValue = getSelfGoal(server, outSerID, true, false);
		if (minValue < VM.fitThreshold) { // 初值已经很小了，没必要在遍历找更优，降低复杂度
			/*VM锁*/
			VM.addLock(vmID);
			return myServer;
		}

		/*搜索除自己以外的其他服务器*/
		int betValue;
		cyt::sServerItem betServer;
		bool findFlag;
		tie(betServer, betValue, findFlag) = findMigInSer(server, requestVM.needCPU/2, requestVM.needRAM/2,
			requestVM.needCPU/2, requestVM.needRAM/2, delSerSet, true, false, outSerID, false);
		// 保存搜索结果，用于快速判断，提高运行速度	
		updateGoalSaver(server, VM, delSerSet, requestVM, vmName, true, outSerID, false, betServer, betValue, findFlag);

		/*自己服务器的得分 vs 其他服务器的得分*/
		if (betValue < minValue) {
			minValue = betValue;
			myServer = betServer;
		}
		// else
		// 	VM.addLock(vmID);

		/*stopSet*/
		if (!findFlag) {
			VM.addLock(vmID);
		}
	}
	else {  // 单节点

		/*快速判断，提高速度*/
		if (quickFindSingle(myServer, server, VM, vmName, vmID, outSerID, outNode))
			return myServer;

		/*摘除自己的计算初值*/
		minValue = getSelfGoal(server, outSerID, false, outNode);
		if (minValue < VM.fitThreshold) {
			VM.addLock(vmID);
			return myServer;
		}
		
		/*搜索除自己以外的其他服务器 node A && node B*/
		cyt::sServerItem betServerA;
		int betValueA;
		bool findFlagA;
		cyt::sServerItem betServerB;
		int betValueB;
		bool findFlagB;
		cyt::sServerItem betServer;
		int betValue;
		bool findFlag;
		tie(betServerA, betValueA, findFlagA) = findMigInSer(server, requestVM.needCPU, requestVM.needRAM,
			0, 0, delSerSet, false, true, outSerID, outNode);
		tie(betServerB, betValueB, findFlagB) = findMigInSer(server, 0, 0, requestVM.needCPU, 
			requestVM.needRAM, delSerSet, false, false, outSerID, outNode);
		if (betValueA <= betValueB) {
			betValue = betValueA;
			betServer = betServerA;
		}
		else {
			betValue = betValueB;
			betServer = betServerB;
		}
		findFlag = findFlagA || findFlagB;
		// 保存搜索结果，用于快速判断，提高运行速度
		updateGoalSaver(server, VM, delSerSet, requestVM, vmName, false, outSerID, outNode, betServer, betValue, findFlag);

		/*自己服务器的得分 vs 其他服务器的得分*/
		if (betValue < minValue) {
			minValue = betValue;
			myServer = betServer;
		}
		// else
		// 	VM.addLock(vmID);

		/*stopSet*/
		if (!findFlag) {
			VM.addLock(vmID);
		}
	}

	return myServer;
}

int getGoal(cSSP_Mig_Server &server, int SerID, bool isDouble, bool node, int needCPU, int needRAM) {
/* Fn: 计算迁入服务器的得分，用来选择迁入服务器
*
* Note:
*	- 如果 计算迁出服务器自己的得分 needCPU=0 needRAM=0 即可
*	- 双节点虚拟机 node参数没有用
*/
	sMyEachServer tempServer = server.myServerSet[SerID];

	int restCPU, restRAM;
	if (isDouble) { // double
		restCPU = tempServer.aIdleCPU + tempServer.bIdleCPU - needCPU;
		restRAM = tempServer.aIdleRAM + tempServer.bIdleRAM - needRAM;
	}
	else {
		if (node) { // node a
			restCPU = tempServer.aIdleCPU - needCPU;
			restRAM = tempServer.aIdleRAM - needRAM;
		}
		else {// node b
			restCPU = tempServer.bIdleCPU - needCPU;
			restRAM = tempServer.bIdleRAM - needRAM;
		}
	}

	int tempValue1 = restCPU + restRAM + abs(restCPU - server.args[2] * restRAM) * server.args[3];
	// int tempValue2 = server.info[tempServer.serName].energyCost;
	// int tempValue = tempValue1 * tempValue2 / server.serverVMSet[SerID].size();
	// int tempValue = tempValue1 * tempValue2 * weightChart(server.serverVMSet[SerID].size());

	return tempValue1;

}

int getSelfGoal(cSSP_Mig_Server &server, int SerID, bool isDouble, bool node) {
/* Fn: 计算自己得分，已摘除自己
*/
	return getGoal(server, SerID, isDouble, node, 0, 0);
}

bool quickFindDouble(cyt::sServerItem &myServer, cSSP_Mig_Server &server, cSSP_Mig_VM &VM, string vmName, string vmID, int outSerID) {
/* Fn: 利用saveGoal快速搜索，空间换时间
* Out:
*	- true 可以快速搜索，false 需要进入正常搜索
*	- 如果是搜索到结果 结果记录在myServer中
*/
	if (VM.saveGoal.count(vmName)) {
		if (VM.saveGoal.at(vmName).goal == -1) { // 找不到可以迁入的服务器
			VM.addLock(vmID); // 加锁
			return true;
		}
		else if (VM.saveGoal[vmName].serID != outSerID) { // 找到了 但不是自己 就和自己当前的得分比较
			int tempValue = getSelfGoal(server, outSerID, true, false);
			if (tempValue < VM.saveGoal[vmName].goal) {
				myServer.hardCost = 1;
				VM.addLock(vmID);
			}
			else {
				myServer.energyCost = -1;
				myServer.hardCost = -1;
				myServer.buyID = VM.saveGoal[vmName].serID;
			}
			return true;
		}
	}

	return false;
}

bool quickFindSingle(cyt::sServerItem &myServer, cSSP_Mig_Server &server, cSSP_Mig_VM &VM, string vmName, string vmID, 
	int outSerID, bool outNode) {
/* Fn: 同上
*/
	if (VM.saveGoal.count(vmName)) {
		if (VM.saveGoal.at(vmName).goal == -1) {
			VM.addLock(vmID);
			return true;
		}
		else if (VM.saveGoal[vmName].serID != outSerID || VM.saveGoal[vmName].node != outNode) { // 不是 同服务器同节点
			int tempValue = getSelfGoal(server, outSerID, false, outNode);
			if (tempValue < VM.saveGoal[vmName].goal) {
				myServer.hardCost = 1;
				VM.addLock(vmID);
			}
			else {
				myServer.energyCost = -1;
				myServer.hardCost = -1;
				myServer.buyID = VM.saveGoal[vmName].serID;
				myServer.node = VM.saveGoal[vmName].node;
			}
			return true;
		}
	}

	return false;
}

map<int, map<int, map<int, map<int, vector<int>>>>>::iterator \
greaterEqu1(map<int, map<int, map<int, map<int, vector<int>>>>> &set, int val) {
	map<int, map<int, map<int, map<int, vector<int>>>>>::iterator it = set.upper_bound(val);
	if (it == set.begin()) {
		return it;
	}
	else if ((--it)->first < val) {
		return ++it;
	}
	else
		return it;
}

map<int, map<int, map<int, vector<int>>>>::iterator \
greaterEqu2(map<int, map<int, map<int, vector<int>>>> &set, int val) {
	map<int, map<int, map<int, vector<int>>>>::iterator it = set.upper_bound(val);
	if (it == set.begin()) {
		return it;
	}
	else if ((--it)->first < val) {
		return ++it;
	}
	else
		return it;
}

map<int, map<int, vector<int>>>::iterator \
greaterEqu3(map<int, map<int, vector<int>>> &set, int val) {
	map<int, map<int, vector<int>>>::iterator it = set.upper_bound(val);
	if (it == set.begin()) {
		return it;
	}
	else if ((--it)->first < val) {
		return ++it;
	}
	else
		return it;
}

map<int, vector<int>>::iterator \
greaterEqu4(map<int, vector<int>> &set, int val) {
	map<int, vector<int>>::iterator it = set.upper_bound(val);
	if (it == set.begin()) {
		return it;
	}
	else if ((--it)->first < val) {
		return ++it;
	}
	else
		return it;
}

tuple<cyt::sServerItem, int, bool> findMigInSer(cSSP_Mig_Server &server, int needCPUa, int needRAMa, int needCPUb, int needRAMb, 
	unordered_map<int, sMyEachServer> &delSerSet, bool isDouble, bool inNode, int outSerID, bool outNode) {
/* Fn: 4 map 嵌套搜索，不考虑迁出服务器自己的得分
*
* Out:
*	- 搜索结果，int:最优得分（越小越好）
*	- bool: 是否能够找到任意一台其他服务器 可以装载这个虚拟机
*
* Note:
*	- isDouble == true时，inNode outNode参数没有用
*	- isDouble == false时，这个函数只能搜索node a或者node b (node参数)，因此全部搜索需要调用两次
*/
	bool findFlag = false;

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

						/*不考虑迁出服务器本身*/
						if (isDouble) {
							if (outSerID == inSerID) 
								continue;
						}
						else {
							if (outSerID == inSerID && inNode == outNode)
								continue;
						}

						/*如果inSerID已经空了，那就不选择这台服务器*/
						if (!server.isOpen(inSerID))
							continue;

						/*后迁移，需要考虑当日删除vm的影响*/
						sMyEachServer tempServer;
						if (delSerSet.count(inSerID) == 1) {   // 该服务器在当天有删除操作
							tempServer = delSerSet[inSerID];
						}
						else {  // 表示这台服务器当天没有删除操作
							tempServer = server.myServerSet[inSerID];  // 既然要根据虚拟机来，排序就没有用了，遍历所有服务器
						}
						if (isDouble) {
							if (tempServer.aIdleCPU < needCPUa || tempServer.bIdleCPU < needCPUb ||
								tempServer.aIdleRAM < needRAMa || tempServer.bIdleRAM < needRAMb) {
								continue;
							}
						}
						else {
							if (inNode == true) {
								if (tempServer.aIdleCPU < needCPUa || tempServer.aIdleRAM < needRAMa) {
									continue;
								}
							}
							else {
								if (tempServer.bIdleCPU < needCPUb || tempServer.bIdleRAM < needRAMb) {
									continue;
								}
							}
						}

						/*计算得分*/
						int tempValue = getGoal(server, inSerID, isDouble, inNode, needCPUa + needCPUb, needRAMa + needRAMb);
						if (tempValue < ompMinValue[omp_get_thread_num()]) {
							findFlag = true;
							ompMinValue[omp_get_thread_num()] = tempValue;
							ompServer[omp_get_thread_num()].energyCost = -1;
							ompServer[omp_get_thread_num()].hardCost = -1;
							if (!isDouble) {
								ompServer[omp_get_thread_num()].node = inNode;
							}
							ompServer[omp_get_thread_num()].buyID = inSerID;   // 记录服务器

						}
					}
				}
			}
		}
	}
	/*线程合并结果*/
	cyt::sServerItem betServer;
	int betValue;
	if (ompMinValue[0] <= ompMinValue[1]) {
		betServer = ompServer[0];
		betValue = ompMinValue[0];
	}
	else {
		betServer = ompServer[1];
		betValue = ompMinValue[1];
	}

	return make_tuple(betServer, betValue, findFlag);
}

void updateGoalSaver(cSSP_Mig_Server &server, cSSP_Mig_VM &VM, unordered_map<int, sMyEachServer> &delSerSet, 
	sVmItem &requestVM, string vmName, bool isDouble, int outSerID, bool outNode, 
	cyt::sServerItem &betServer, int betValue, bool findFlag) {
/* Fn: 记录某种型号vm 对应的 4map遍历结果 用于提高速度
*
* In: 如果 isDouble==true，outNode就没用到
*
* Note:
*	- 数据储存于 VM.saveGoal中
*	- 当执行迁移操作时，VM.saveGoal清空
*	- 因为4map遍历时，没有统计vm迁出服务器未摘除自己的结果，需要补充上（同型号的vm寻找迁入服务器时是需要考虑这个结果的）
*/
	sMyEachServer tempServer;
	int tempValue = 0;

	/*去掉删除的影响*/
	if (delSerSet.count(outSerID))
		tempServer = delSerSet[outSerID];
	else
		tempServer = server.myServerSet[outSerID];

	/*判断 如果不摘除自己 能不能装进去*/
	if (isDouble) {
		if (tempServer.aIdleCPU < requestVM.needCPU / 2 || tempServer.bIdleCPU < requestVM.needCPU / 2 ||
			tempServer.aIdleRAM < requestVM.needRAM / 2 || tempServer.bIdleRAM < requestVM.needRAM / 2)
			tempValue = -1; // 装不进去
	}
	else {
		if (outNode) {
			if (tempServer.aIdleCPU < requestVM.needCPU || tempServer.aIdleRAM < requestVM.needRAM)
				tempValue = -1; // 装不进去
		}
		else {
			if (tempServer.bIdleCPU < requestVM.needCPU || tempServer.bIdleRAM < requestVM.needRAM)
				tempValue = -1; // 装不进去
		}
	}

	/*如果可以装进去 tempValue==0，计算装进去的得分*/
	if (tempValue == 0) {
		tempValue = getGoal(server, outSerID, isDouble, outNode, requestVM.needCPU, requestVM.needRAM);
	}

	/*需要和4map的搜索结果比较，选择最好的 保存到VM.saveGoal*/
	if (!findFlag && tempValue == -1) {
		sPosGoal oneGoal;
		oneGoal.goal = -1;
		VM.saveGoal.insert({vmName, oneGoal});
	}
	else if (!findFlag && tempValue != -1) {
		sPosGoal oneGoal;
		oneGoal.goal = tempValue;
		oneGoal.serID = outSerID;
		if (!isDouble)
			oneGoal.node = outNode;
		VM.saveGoal.insert({vmName, oneGoal});
	}
	else if (findFlag && tempValue == -1) {
		sPosGoal oneGoal;
		oneGoal.goal = betValue;
		oneGoal.serID = betServer.buyID;
		if (!isDouble)
			oneGoal.node = betServer.node;
		VM.saveGoal.insert({vmName, oneGoal});
	}
	else {
		sPosGoal oneGoal;
		oneGoal.goal = (betValue < tempValue)? betValue : tempValue;
		oneGoal.serID = (betValue < tempValue)? betServer.buyID : outSerID;
		if (!isDouble)
			oneGoal.node = (betValue < tempValue)? betServer.node : outNode;
		VM.saveGoal.insert({vmName, oneGoal});
	}
}
