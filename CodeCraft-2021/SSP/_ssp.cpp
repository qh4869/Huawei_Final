#include "ssp.h"

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
	int tempValue2 = server.info[tempServer.serName].energyCost;
	int tempValue = tempValue1 * tempValue2 / server.serverVMSet[SerID].size();

	return tempValue;

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

tuple<cyt::sServerItem, int, bool> findMigInSer(cSSP_Mig_Server &server, sVmItem &requestVM, 
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

	int needCPUa, needRAMa, needCPUb, needRAMb;
	if (isDouble) {
		needCPUa = requestVM.needCPU / 2;
		needRAMa = requestVM.needRAM / 2;
		needCPUb = requestVM.needCPU / 2;
		needRAMb = requestVM.needRAM / 2;
	}
	else {
		if (inNode) { // node a
			needCPUa = requestVM.needCPU;
			needRAMa = requestVM.needRAM;
			needCPUb = 0;
			needRAMb = 0;
		}
		else {
			needCPUa = 0;
			needRAMa = 0;
			needCPUb = requestVM.needCPU;
			needRAMb = requestVM.needRAM;
		}
	}

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
						int tempValue = getGoal(server, inSerID, isDouble, inNode, requestVM.needCPU, requestVM.needRAM);
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
