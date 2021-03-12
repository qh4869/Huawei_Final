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
	sMyEachServer oneServer;
	oneServer.serName = name;
	oneServer.aIdleCPU = info[name].totalCPU / 2;
	oneServer.bIdleCPU = info[name].totalCPU / 2;
	oneServer.aIdleRAM = info[name].totalRAM / 2;
	oneServer.bIdleRAM = info[name].totalRAM / 2;

	myServerSet.push_back(oneServer);

	if (buyRecord[iDay].count(name)) 
		buyRecord[iDay][name]++;
	else {
		buyRecord[iDay].insert(std::make_pair(name, 1));
		newServerNum[iDay]++;
	}

	return myServerSet.size()-1;

}

int cServer::firstFitDouble(int reqCPU, int reqRAM) {
/* Fn: 双节点部署
*/
	for (int iSer=0; iSer<myServerSet.size(); iSer++) {
		if (myServerSet[iSer].aIdleCPU > reqCPU/2 && myServerSet[iSer].bIdleCPU > reqCPU/2 \
			&& myServerSet[iSer].aIdleRAM > reqRAM/2 && myServerSet[iSer].bIdleRAM > reqRAM/2)
			return iSer;
	}

	return -1;
}

std::tuple<int, bool> cServer::firstFitSingle(int reqCPU, int reqRAM) {
/* Fn: 单节点部署，bool输入为true表示A节点
*/
	for (int iSer=0; iSer<myServerSet.size(); iSer++) {
		if (myServerSet[iSer].aIdleCPU > reqCPU && myServerSet[iSer].aIdleRAM > reqRAM)
			return make_tuple(iSer, true);
		if (myServerSet[iSer].bIdleCPU > reqCPU && myServerSet[iSer].bIdleRAM > reqRAM)
			return make_tuple(iSer, false);
	}

	return make_tuple(-1, false);
}

bool cServer::mycomp(pair<string, int> i, pair<string, int> j) {
    return i.second < j.second;
}

void cServer::rankServerByPrice() {
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