#include "Server.h"
#include <iostream>

int cServer::purchase(string name, int iDay) {
/* In:
*	- name: 服务器型号
* 	- iDay: 第几天
*	
* Out:
*	- 购买的服务器ID
*/
	/*add into myServerSet*/
	sMyEachServer oneServer;
	oneServer.serName = name;
	oneServer.aIdleCPU = info[name].totalCPU / 2;
	oneServer.bIdleCPU = info[name].totalCPU / 2;
	oneServer.aIdleRAM = info[name].totalRAM / 2;
	oneServer.bIdleRAM = info[name].totalRAM / 2;
	myServerSet.push_back(oneServer);

	/*add into serverVMSet-empty*/
	unordered_map<string, int> vmSet;
	serverVMSet.push_back(vmSet);

	/*add into buyRecord*/
	if (!buyRecord[iDay].count(name)) { // 当天没买过这个类型
		buyRecord[iDay].insert(make_pair(name, 1));
	}
	else { // 当天买过这个类型
		buyRecord[iDay][name]++;
	}

	return myServerSet.size()-1;
}

int cServer::firstFitDouble(int reqCPU, int reqRAM) {
/* Fn: 双节点部署
*/
	for (int iSer=0; iSer<(int)myServerSet.size(); iSer++) {
		if (myServerSet[iSer].aIdleCPU > reqCPU/2 && myServerSet[iSer].bIdleCPU > reqCPU/2 \
			&& myServerSet[iSer].aIdleRAM > reqRAM/2 && myServerSet[iSer].bIdleRAM > reqRAM/2)
			return iSer;
	}

	return -1;
}

std::tuple<int, bool> cServer::firstFitSingle(int reqCPU, int reqRAM) {
/* Fn: 单节点部署，bool输入为true表示A节点
*/
	for (int iSer=0; iSer<(int)myServerSet.size(); iSer++) {
		if (myServerSet[iSer].aIdleCPU > reqCPU && myServerSet[iSer].aIdleRAM > reqRAM)
			return make_tuple(iSer, true);
		if (myServerSet[iSer].bIdleCPU > reqCPU && myServerSet[iSer].bIdleRAM > reqRAM)
			return make_tuple(iSer, false);
	}

	return make_tuple(-1, false);
}

int cServer::firstFitByIdleOrdDouble(int reqCPU, int reqRAM) {
/* Fn: 待优化：不需要每次对全部排序，而是被部署的服务器调整，新买的服务器调整,遗留
*/
	/*优先装空闲总量最少的服务器*/
	rankMyServerbyIdle(); // 先排序
	int serID;
	for (auto it=myServerOrder.begin(); it!=myServerOrder.end(); it++) {
		serID = it->first;
		if (myServerSet[serID].aIdleCPU > reqCPU/2 && myServerSet[serID].bIdleCPU > reqCPU/2 \
			&& myServerSet[serID].aIdleRAM > reqRAM/2 && myServerSet[serID].bIdleRAM > reqRAM/2)
			return serID;
	}

	return -1;
}

std::tuple<int, bool> cServer::firstFitByIdleOrdSingle(int reqCPU, int reqRAM) {
	/*优先装空闲总量最少的服务器*/
	rankMyServerbyIdle(); // 先排序
	int serID;
	for (auto it=myServerOrder.begin(); it!=myServerOrder.end(); it++) {
		serID = it->first;
		if (myServerSet[serID].aIdleCPU > reqCPU && myServerSet[serID].aIdleRAM > reqRAM)
			return make_tuple(serID, true);
		if (myServerSet[serID].bIdleCPU > reqCPU && myServerSet[serID].bIdleRAM > reqRAM)
			return make_tuple(serID, false);
	}

	return make_tuple(-1, false);
}


bool cServer::mycomp(pair<string, int> i, pair<string, int> j) {
    return i.second < j.second;
}

bool cServer::mycompID(pair<int, int> i, pair<int, int> j) {
	return i.second < j.second;
}

bool cServer::mycompIDDB(pair<int, double> i, pair<int, double> j) {
	return i.second < j.second;
}

void cServer::rankServerByPrice() {
/* Fn: 所有服务器类型的排序
*/
	priceOrder.clear();
	for (auto ite=info.begin(); ite!=info.end(); ite++) { // 迭代器
		priceOrder.push_back(make_pair(ite->first, ite->second.hardCost + ite->second.energyCost * alpha));
	}

	sort(priceOrder.begin(), priceOrder.end(), cServer::mycomp);
}

string cServer::chooseSer(int reqCPU, int reqRAM, bool isDoubleNode) {
	string serName;

	if (isDoubleNode) {
		for (auto ite=priceOrder.begin(); ite!=priceOrder.end(); ite++) {
			serName = ite->first;
			if (info[serName].totalCPU > reqCPU && info[serName].totalRAM > reqRAM)
				return serName;
		}
	}
	else {
		for (auto ite=priceOrder.begin(); ite!=priceOrder.end(); ite++) {
			serName = ite->first;
			if (info[serName].totalCPU/2 > reqCPU && info[serName].totalRAM/2 > reqRAM)
				return serName;
		}
	}

	return "";
}

bool cServer::isOpen(int serID) {
	string serName = myServerSet[serID].serName;

	if (myServerSet[serID].aIdleCPU == info[serName].totalCPU/2 \
		&& myServerSet[serID].bIdleCPU == info[serName].totalCPU/2 \
		&& myServerSet[serID].aIdleRAM == info[serName].totalRAM/2 \
		&& myServerSet[serID].bIdleRAM == info[serName].totalRAM/2)
		return false;
	return true;
}

int cServer::idMapping() {
/* Fn: record中的 server id是按照购买顺序记录的，而输出要求是同一种类型的服务器一起编号的
*		所以要一个idMap表，用来记录新旧id的映射关系
*/
	int realID = 0;
	string serName;
	int startID = 0; // 每天服务器id号 起始值
	int serNum;
	int cnt; // 某个类型已经找到几台
	int oldIDmax = 0; // debug用，某一天的最大old id号

	for (const auto &xDayRecord : buyRecord) { // 每天
		for (const auto &buyItem : xDayRecord) { // 每种服务器类型
			serName = buyItem.first;
			serNum = buyItem.second; // 当天这个型号购买个数
			cnt = 0;
			for (int oldID=startID; cnt!=serNum; oldID++) {
				if (serName == myServerSet[oldID].serName) { // 找到
					idMap.insert(make_pair(oldID, realID));
					oldIDmax = (oldID > oldIDmax) ? oldID : oldIDmax;
					realID++;
					cnt++;
				}
			}
		}
		startID = realID;
		if (oldIDmax + 1 != realID) {
			cout << "购买记录和服务器库存不一致" << endl;
			return -1;
		}
	}

	return 0;
}

void cServer::rankMyServerbyIdle() {
/* Fn: 按照空闲资源的顺序对已有的服务器排序，结果装入myServerOrder <vmName, idle resource>
*	- 四种资源直接求和得到结果
*/
	myServerOrder.clear();
	for (int iSer=0; iSer<(int)myServerSet.size(); iSer++) {
		myServerOrder.push_back(make_pair(iSer, myServerSet[iSer].aIdleCPU + myServerSet[iSer].aIdleRAM \
			+ myServerSet[iSer].bIdleCPU + myServerSet[iSer].bIdleRAM));
	}

	sort(myServerOrder.begin(), myServerOrder.end(), cServer::mycompID);
}

void cServer::rankMyServerbyOccupied() {
/* Fn: 按照占有资源的顺序排序，结果装入 "同上"
*/
	int ocpResource;
	string serName;

	myServerOrder.clear();
	for (int iSer=0; iSer<(int)myServerSet.size(); iSer++) {
		serName = myServerSet[iSer].serName;
		ocpResource = info[serName].totalCPU + info[serName].totalRAM \
			- myServerSet[iSer].aIdleCPU - myServerSet[iSer].aIdleRAM \
			- myServerSet[iSer].bIdleCPU - myServerSet[iSer].bIdleRAM;
		myServerOrder.push_back(make_pair(iSer, ocpResource));
	}

	sort(myServerOrder.begin(), myServerOrder.end(), cServer::mycompID);
}

void cServer::rankMyserverbyRatio() {
/* Fn: 按照空闲占比从小到大，结果装入 myServerOrderDB
*/
	double ratio;
	string serName;

	myServerOrderDB.clear();
	for (int iSer=0; iSer<(int)myServerSet.size(); iSer++) {
		serName = myServerSet[iSer].serName;
		ratio = (myServerSet[iSer].aIdleCPU + myServerSet[iSer].aIdleRAM \
			+ myServerSet[iSer].bIdleCPU + myServerSet[iSer].bIdleRAM) \
			/ (info[serName].totalCPU + info[serName].totalRAM);
		myServerOrderDB.push_back(make_pair(iSer, ratio));
	}

	sort(myServerOrderDB.begin(), myServerOrderDB.end(), cServer::mycompIDDB);
}

void cServer::genInfoV() {
	for (const auto &xSer : info) 
		infoV.push_back(make_pair(xSer.first, xSer.second));
}

bool cyt::mycompID(pair<int, int> i, pair<int, int> j) {
	return i.second <= j.second;
}

// 对服务器里的虚拟机数量进行排序
void cServer::updatVmSourceOrder(int needCPU, int needRAM, int serID, bool flag) {  // true : add , false : delete

	int pos;   // serID对应的服务器在vmNumOrder中的位置
	if (vmSourceOrder.size() == 0) {    // 初始化
		int value = needCPU + needRAM;
		vmSourceOrder.push_back(make_pair(serID, value));   // 最开始直接初始化
		return;
	}
	else if (serID > (int)vmSourceOrder.size() - 1) {   // 新加入的服务器
		if (!flag) {
			cout << "sort error" << endl;
		}
		vmSourceOrder.insert(vmSourceOrder.begin(), make_pair(serID, 0));   // 将新加入的插进第一个位置，初始值为0
		pos = 0;
	}
	else {
		for (int i = 0; i < (int)vmSourceOrder.size(); i++) {
			if (vmSourceOrder[i].first == serID) {   // 找到对应的位置
				pos = i;
				break;
			}
		}
	}


	if (!flag && pos == 0) {  // 删除并且处于第一位，则不需要移动
		vmSourceOrder[pos].second -= (needCPU + needRAM);   // 减去删除的虚拟机资源
	}
	else {
		pair<int, int> temp = vmSourceOrder[pos];    // 取出该变量
		if (flag) {    // true : add
			temp.second += (needCPU + needRAM);
		}
		else {   // false : delete
			temp.second -= (needCPU + needRAM);
		}
		auto ite = vmSourceOrder.begin();
		if (pos - 1 < 0 || cyt::mycompID(vmSourceOrder[pos - 1], temp)) {  // pos - 1 < 0表示是第一个元素
			if (pos != (int)vmSourceOrder.size() - 1) {   // 不是最后一个元素
				for (int i = pos + 1; i < (int)vmSourceOrder.size(); i++) {
					if (i == (int)vmSourceOrder.size() - 1 && cyt::mycompID(vmSourceOrder[i], temp)) {  // 最后一个元素还是小
						vmSourceOrder.erase(ite + pos);
						vmSourceOrder.push_back(temp);
						break;
					}
					if (cyt::mycompID(temp, vmSourceOrder[i])) {   // 往下找到了比自己大的，要插在这个比自己大的前面
						vmSourceOrder.erase(ite + pos);   // 删除该元素
						vmSourceOrder.insert(vmSourceOrder.begin() + i - 1, temp);   // 在指定的位置插入元素
						break;   // 退出
					}
				}
			}
			else {
				vmSourceOrder[pos] = temp;   // 赋予新值
			}
		}
		else {
			for (int i = pos - 1; i >= 0; i--) {   // 走到这里说明了pos - 1 > 0
				if (i == 0 && cyt::mycompID(temp, vmSourceOrder[i])) {
					vmSourceOrder.erase(ite + pos);
					vmSourceOrder.insert(vmSourceOrder.begin() + i, temp);
					break;
				}
				if (cyt::mycompID(vmSourceOrder[i], temp)) {
					vmSourceOrder.erase(ite + pos);
					vmSourceOrder.insert(vmSourceOrder.begin() + i + 1, temp);
					break;
				}
			}
		}
	}

}

void cServer::updatVmTarOrder(int needCPUa, int needRAMa, int needCPUb, int needRAMb, int serID, bool flag) {  // true : add , false : delete

	string serName = myServerSet[serID].serName;
	int totalCPU = info[serName].totalCPU;
	int totalRAM = info[serName].totalRAM;
	int aIdleCPU = myServerSet[serID].aIdleCPU;
	int aIdleRAM = myServerSet[serID].aIdleRAM;
	int bIdleCPU = myServerSet[serID].bIdleCPU;
	int bIdleRAM = myServerSet[serID].bIdleRAM;
	int alastCPU, alastRAM, blastCPU, blastRAM;

	/*加入新结点*/
	vmTarOrder[aIdleCPU][aIdleRAM][bIdleCPU][bIdleRAM].push_back(serID);
	// if (!vmTarOrder.count(aIdleCPU)) {
	// 	vmTarOrder.insert({aIdleCPU, {{aIdleRAM, {{bIdleCPU, {{bIdleRAM, {serID} }} }} }} });
	// }
	// else {
	// 	if (!vmTarOrder[aIdleCPU].count(aIdleRAM)) {
	// 		vmTarOrder[aIdleCPU].insert({aIdleRAM, {{bIdleCPU, {{bIdleRAM, {serID} }} }} });
	// 	}
	// 	else {
	// 		if (!vmTarOrder[aIdleCPU][aIdleRAM].count(bIdleCPU)) {
	// 			vmTarOrder[aIdleCPU][aIdleRAM].insert({bIdleCPU, {{bIdleRAM, {serID} }} });
	// 		}
	// 		else {
	// 			if (!vmTarOrder[aIdleCPU][aIdleRAM][bIdleCPU].count(bIdleRAM)) {
	// 				vmTarOrder[aIdleCPU][aIdleRAM][bIdleCPU].insert({bIdleRAM, {serID} });
	// 			}
	// 			else {
	// 				vmTarOrder[aIdleCPU][aIdleRAM][bIdleCPU][bIdleRAM].push_back(serID);
	// 			}
	// 		}
	// 	}			
	// }

	if (flag) {
		alastCPU = aIdleCPU + needCPUa;
		alastRAM = aIdleRAM + needRAMa;
		blastCPU = bIdleCPU + needCPUb;
		blastRAM = bIdleRAM + needRAMb;
	}
	else {
		alastCPU = aIdleCPU - needCPUa;
		alastRAM = aIdleRAM - needRAMa;
		blastCPU = bIdleCPU - needCPUb;
		blastRAM = bIdleRAM - needRAMb;
	}

	/*删除旧节点*/
	if (vmTarOrder.count(alastCPU) && vmTarOrder[alastCPU].count(alastRAM) \
		&& vmTarOrder[alastCPU][alastRAM].count(blastCPU) && vmTarOrder[alastCPU][alastRAM][blastCPU].count(blastRAM)) {
		vector<int> &serSet = vmTarOrder[alastCPU][alastRAM][blastCPU][blastRAM];
		for (auto it=serSet.begin(); it!=serSet.end(); it++) {
			if (*it==serID) {
				serSet.erase(it);
				break;
			}
		}
		if (serSet.empty()) {
			vmTarOrder[alastCPU][alastRAM][blastCPU].erase(blastRAM);
			if (vmTarOrder[alastCPU][alastRAM][blastCPU].empty()) {
				vmTarOrder[alastCPU][alastRAM].erase(blastCPU);
				if (vmTarOrder[alastCPU][alastRAM].empty()) {
					vmTarOrder[alastCPU].erase(alastRAM);
					if (vmTarOrder[alastCPU].empty()) {
						vmTarOrder.erase(alastCPU);
					}
				}
			}
		}
	}
}

void cServer::updatVmTarOrderNodeA(int needCPU, int needRAM, int serID, bool flag) {
	string serName = myServerSet[serID].serName;
	int aIdleCPU = myServerSet[serID].aIdleCPU;
	int aIdleRAM = myServerSet[serID].aIdleRAM;
	int alastCPU, alastRAM;

	/*加入新结点*/
	vmTarOrderNodeA[aIdleCPU][aIdleRAM].push_back(serID);

	if (flag) {
		alastCPU = aIdleCPU + needCPU;
		alastRAM = aIdleRAM + needRAM;
	}
	else {
		alastCPU = aIdleCPU - needCPU;
		alastRAM = aIdleRAM - needRAM;
	}

	/*删除旧节点*/
	if (vmTarOrderNodeA.count(alastCPU) && vmTarOrderNodeA[alastCPU].count(alastRAM)) {
		vector<int> &serSet = vmTarOrderNodeA[alastCPU][alastRAM];
		for (auto it=serSet.begin(); it!=serSet.end(); it++) {
			if (*it==serID) {
				serSet.erase(it);
				break;
			}
		}
		if (serSet.empty()) {
			vmTarOrderNodeA[alastCPU].erase(alastRAM);
			if (vmTarOrderNodeA[alastCPU].empty()) {
				vmTarOrderNodeA.erase(alastCPU);
			}
		}
	}
}

void cServer::updatVmTarOrderNodeB(int needCPU, int needRAM, int serID, bool flag) {
	string serName = myServerSet[serID].serName;
	int bIdleCPU = myServerSet[serID].bIdleCPU;
	int bIdleRAM = myServerSet[serID].bIdleRAM;
	int blastCPU, blastRAM;

	/*加入新结点*/
	vmTarOrderNodeB[bIdleCPU][bIdleRAM].push_back(serID);

	if (flag) {
		blastCPU = bIdleCPU + needCPU;
		blastRAM = bIdleRAM + needRAM;
	}
	else {
		blastCPU = bIdleCPU - needCPU;
		blastRAM = bIdleRAM - needRAM;
	}

	/*删除旧节点*/
	if (vmTarOrderNodeB.count(blastCPU) && vmTarOrderNodeB[blastCPU].count(blastRAM)) {
		vector<int> &serSet = vmTarOrderNodeB[blastCPU][blastRAM];
		for (auto it=serSet.begin(); it!=serSet.end(); it++) {
			if (*it==serID) {
				serSet.erase(it);
				break;
			}
		}
		if (serSet.empty()) {
			vmTarOrderNodeB[blastCPU].erase(blastRAM);
			if (vmTarOrderNodeB[blastCPU].empty()) {
				vmTarOrderNodeB.erase(blastCPU);
			}
		}
	}
}

void cServer::updatVmTarOrderDouble(int needCPU, int needRAM, int serID, int lastCPU, int lastRAM) {
/* In:
*	- needCPU/needRAM指的是单节点的值
*/
	string serName = myServerSet[serID].serName;
	int aIdleCPU = myServerSet[serID].aIdleCPU;
	int aIdleRAM = myServerSet[serID].aIdleRAM;
	int bIdleCPU = myServerSet[serID].bIdleCPU;
	int bIdleRAM = myServerSet[serID].bIdleRAM;
	// int alastCPU, alastRAM, blastCPU, blastRAM;

	int minIdleCPU = (aIdleCPU < bIdleCPU) ? aIdleCPU : bIdleCPU;
	int minIdleRAM = (aIdleRAM < bIdleRAM) ? aIdleRAM : bIdleRAM;

	/*加入新结点*/
	vmTarOrderDouble[minIdleCPU][minIdleRAM].push_back(serID);

	/*删除节点*/
	if (lastCPU != INT_MAX || lastRAM != INT_MAX) { // 第一次部署不需要删除

	}


}