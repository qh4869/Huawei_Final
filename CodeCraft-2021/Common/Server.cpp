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

void cServer::updateResourceRatio() {
/* Fn: 统计每天资源占空比，如果没有服务器或者所有服务器都空，push -1
* 
*/
	int totalCPUadd = 0;
	int totalRAMadd = 0;
	int occCPUadd = 0;
	int occRAMadd = 0;

	for (auto &x : myServerSet) {
		string serName = x.serName;
		int totalCPU = info[serName].totalCPU;
		int totalRAM = info[serName].totalRAM;
		int occCPU = totalCPU - x.aIdleCPU - x.bIdleCPU;
		int occRAM = totalRAM - x.aIdleRAM - x.bIdleRAM;

		if (occCPU == 0 && occRAM == 0) // 是空的服务器就跳过
			continue;

		totalCPUadd += totalCPU;
		totalRAMadd += totalRAM;
		occCPUadd += occCPU;
		occRAMadd += occRAM;
	}

	if (totalCPUadd == 0 && totalRAMadd == 0) { // 所有服务器全是空的，或者一个服务器也没有
		cpuRatio.push_back(-1);
		ramRatio.push_back(-1);
	}
	else {
		cpuRatio.push_back( (double)occCPUadd / totalCPUadd );
		ramRatio.push_back( (double)occRAMadd / totalRAMadd );
	}
}

void cServer::updateResourceRatioIncludeALL() {
/* Fn: 功能同上，不跳过空的服务器
*/
	int totalCPUadd = 0;
	int totalRAMadd = 0;
	int occCPUadd = 0;
	int occRAMadd = 0;

	for (auto &x : myServerSet) {
		string serName = x.serName;
		int totalCPU = info[serName].totalCPU;
		int totalRAM = info[serName].totalRAM;
		int occCPU = totalCPU - x.aIdleCPU - x.bIdleCPU;
		int occRAM = totalRAM - x.aIdleRAM - x.bIdleRAM;

		totalCPUadd += totalCPU;
		totalRAMadd += totalRAM;
		occCPUadd += occCPU;
		occRAMadd += occRAM;
	}

	if (totalCPUadd == 0 && totalRAMadd == 0) {// 一个服务器都没有
		cpuRatioIncAll.push_back(-1);
		ramRatioIncAll.push_back(-1);
	}
	else {
		cpuRatioIncAll.push_back((double)occCPUadd / totalCPUadd);
		ramRatioIncAll.push_back((double)occRAMadd / totalRAMadd);
	}
}

tuple<double, double> cServer::getAveResourceRatio() {
/* Fn: 返回资源占比的均值，如果vector为空（第0天，或者任何一天都没有有效值），返回-1 -1
* Note:
*	- cpuCnt 和 ramCnt 应该是相等的
*/
	double cpuAdd = 0;
	int cpuCnt = 0;
	double ramAdd = 0;
	int ramCnt = 0;

	for (auto &x : cpuRatio) {
		if (x == -1)
			continue;
		else {
			cpuAdd += x;
			cpuCnt++;
		}
	}
	for (auto &x : ramRatio) {
		if (x == -1)
			continue;
		else {
			ramAdd += x;
			ramCnt++;
		}
	}

	if (cpuCnt != 0 && ramCnt != 0)
		return {cpuAdd / cpuCnt, ramAdd / ramCnt};
	else
		return {-1, -1};
}

tuple<double, double> cServer::getAveResourceRatioIncludeALL() {
/* Fn: 类似同上 cpuRatioIncAll ramRatioIncAll
*/
	double cpuAdd = 0;
	int cpuCnt = 0;
	double ramAdd = 0;
	int ramCnt = 0;

	for (auto &x : cpuRatioIncAll) {
		if (x == -1)
			continue;
		else {
			cpuAdd += x;
			cpuCnt++;
		}
	}
	for (auto &x : ramRatioIncAll) {
		if (x == -1)
			continue;
		else {
			ramAdd += x;
			ramCnt++;
		}
	}

	if (cpuCnt != 0 && ramCnt != 0)
		return {cpuAdd / cpuCnt, ramAdd / ramCnt};
	else
		return {-1, -1};
}