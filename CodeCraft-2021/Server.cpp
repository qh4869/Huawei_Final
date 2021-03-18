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

	/*add into buyRecord*/
	if (!buyRecord[iDay].count(name)) { // 当天没买过这个类型
		buyRecord[iDay].insert(make_pair(name, 1));
	}
	else { // 当天买过这个类型
		buyRecord[iDay][name]++;
	}

	return myServerSet.size() - 1;
}

int cServer::firstFitDouble(int reqCPU, int reqRAM) {
	/* Fn: 双节点部署
	*/
	for (int iSer = 0; iSer<(int)myServerSet.size(); iSer++) {
		if (myServerSet[iSer].aIdleCPU > reqCPU / 2 && myServerSet[iSer].bIdleCPU > reqCPU / 2 \
			&& myServerSet[iSer].aIdleRAM > reqRAM / 2 && myServerSet[iSer].bIdleRAM > reqRAM / 2)
			return iSer;
	}

	return -1;
}

std::tuple<int, bool> cServer::firstFitSingle(int reqCPU, int reqRAM) {
	/* Fn: 单节点部署，bool输入为true表示A节点
	*/
	for (int iSer = 0; iSer<(int)myServerSet.size(); iSer++) {
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
	for (auto it = myServerOrder.begin(); it != myServerOrder.end(); it++) {
		serID = it->first;
		if (myServerSet[serID].aIdleCPU > reqCPU / 2 && myServerSet[serID].bIdleCPU > reqCPU / 2 \
			&& myServerSet[serID].aIdleRAM > reqRAM / 2 && myServerSet[serID].bIdleRAM > reqRAM / 2)
			return serID;
	}

	return -1;
}

std::tuple<int, bool> cServer::firstFitByIdleOrdSingle(int reqCPU, int reqRAM) {
	/*优先装空闲总量最少的服务器*/
	rankMyServerbyIdle(); // 先排序
	int serID;
	for (auto it = myServerOrder.begin(); it != myServerOrder.end(); it++) {
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
	for (auto ite = info.begin(); ite != info.end(); ite++) { // 迭代器
		priceOrder.push_back(make_pair(ite->first, ite->second.hardCost + ite->second.energyCost * alpha));
	}

	sort(priceOrder.begin(), priceOrder.end(), cServer::mycomp);
}

string cServer::chooseSer(int reqCPU, int reqRAM, bool isDoubleNode) {
	string serName;

	if (isDoubleNode) {
		for (auto ite = priceOrder.begin(); ite != priceOrder.end(); ite++) {
			serName = ite->first;
			if (info[serName].totalCPU > reqCPU && info[serName].totalRAM > reqRAM)
				return serName;
		}
	}
	else {
		for (auto ite = priceOrder.begin(); ite != priceOrder.end(); ite++) {
			serName = ite->first;
			if (info[serName].totalCPU / 2 > reqCPU && info[serName].totalRAM / 2 > reqRAM)
				return serName;
		}
	}

	return "";
}

bool cServer::isOpen(int serID) {
	string serName = myServerSet[serID].serName;

	if (myServerSet[serID].aIdleCPU == info[serName].totalCPU / 2 \
		&& myServerSet[serID].bIdleCPU == info[serName].totalCPU / 2 \
		&& myServerSet[serID].aIdleRAM == info[serName].totalRAM / 2 \
		&& myServerSet[serID].bIdleRAM == info[serName].totalRAM / 2)
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
			for (int oldID = startID; cnt != serNum; oldID++) {
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
	for (int iSer = 0; iSer<(int)myServerSet.size(); iSer++) {
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
	for (int iSer = 0; iSer<(int)myServerSet.size(); iSer++) {
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
	for (int iSer = 0; iSer<(int)myServerSet.size(); iSer++) {
		serName = myServerSet[iSer].serName;
		ratio = (myServerSet[iSer].aIdleCPU + myServerSet[iSer].aIdleRAM \
			+ myServerSet[iSer].bIdleCPU + myServerSet[iSer].bIdleRAM) \
			/ (info[serName].totalCPU + info[serName].totalRAM);
		myServerOrderDB.push_back(make_pair(iSer, ratio));
	}

	sort(myServerOrderDB.begin(), myServerOrderDB.end(), cServer::mycompIDDB);
}


// 对服务器按价格排序，flag为true表示第一次排序，为false表示不是第一次
void cServer::rankServerByPrice(bool flag) {

	if (flag) {   // 为true表示第一次生成数据，需构造priceOrder
		for (auto ite = info.begin(); ite != info.end(); ite++) { // 迭代器
			priceOrder.push_back(make_pair(ite->first, ite->second.hardCost + ite->second.energyCost * alpha));
		}
	}
	else {   // 之前已经生成过数据了，现在不用重新分配内存
		int i = 0;
		for (auto ite = info.begin(); ite != info.end(); ite++) {
			priceOrder[i] = make_pair(ite->first, ite->second.hardCost + ite->second.energyCost * alpha);
			i++;
		}
	}
	sort(priceOrder.begin(), priceOrder.end(), cServer::mycomp);   // 排序(全部重排)

}