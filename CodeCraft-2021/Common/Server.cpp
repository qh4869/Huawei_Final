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

void cServer::idMappingEachDay(int iDay) {
/* Fn: 每天更新id
*/
	int realID = startID;

	for (auto &buyItem : buyRecord[iDay]) {
		string serName = buyItem.first;
		int serNum = buyItem.second;
		int cnt = 0;
		
		if (serName=="hostUT0CV")
			cout << "";
		for (int oldID = startID; cnt != serNum; oldID++) {
			if (serName == myServerSet[oldID].serName) {
				idMap.insert({oldID, realID});
				realID++;
				cnt++;
			}
		}	
	}

	startID = realID;
}

void cServer::energyCostEachDay() {
	for (int iSer = 0; iSer < (int)myServerSet.size(); iSer++) {
		if (isOpen(iSer)) {
			string vmName = myServerSet[iSer].serName;
			engCostStas += info[vmName].energyCost;
		}
	}
}

void cServer::hardCostTotal() {
	for (int iSer = 0; iSer < (int)myServerSet.size(); iSer++) {
		string vmName = myServerSet[iSer].serName;
		hardCostStas += info[vmName].hardCost;
	}
}

int cServer::getTotalCost() {
	return engCostStas + hardCostStas;
}

void cServer::getMeanPerCost() {

	double totalCPU_1 = 0, totalRAM_1 = 0, totalEnergyCost_1 = 0, totalHardCost_1 = 0;
	double totalCPU_2 = 0, totalRAM_2 = 0, totalEnergyCost_2 = 0, totalHardCost_2 = 0;

	sServerItem serInfo;
	int count = 0;
	for (auto &ite : info) {
		serInfo = ite.second;
		if (count % 2 == 0) {
			totalCPU_1 += serInfo.totalCPU;
			totalRAM_1 += serInfo.totalRAM;
			totalEnergyCost_1 += serInfo.energyCost;
			totalHardCost_1 += serInfo.hardCost;
		}
		else {
			totalCPU_2 += serInfo.totalCPU;
			totalRAM_2 += serInfo.totalRAM;
			totalEnergyCost_2 += serInfo.energyCost;
			totalHardCost_2 += serInfo.hardCost;
		}
		count++;
	}

	RAMHardCost = (totalHardCost_1 * totalCPU_2 - totalHardCost_2 * totalCPU_1) /
		(totalRAM_1 * totalCPU_2 - totalRAM_2 * totalCPU_1);
	CPUHardCost = (totalHardCost_1 * totalRAM_2 - totalHardCost_2 * totalRAM_1) /
		(totalCPU_1 * totalRAM_2 - totalCPU_2 * totalRAM_1);
	RAMEnergyCost = (totalEnergyCost_1 * totalCPU_2 - totalEnergyCost_2 * totalCPU_1) /
		(totalRAM_1 * totalCPU_2 - totalRAM_2 * totalCPU_1);
	CPUEnergyCost = (totalEnergyCost_1 * totalRAM_2 - totalEnergyCost_2 * totalRAM_1) /
		(totalCPU_1 * totalRAM_2 - totalCPU_2 * totalRAM_1);

}