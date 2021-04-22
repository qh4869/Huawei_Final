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

void cServer::idMappingEachDay(int iDay) {
/* Fn: 每天更新id
*/
	int realID = startID;

	for (auto &buyItem : buyRecord[iDay]) {
		string serName = buyItem.first;
		int serNum = buyItem.second;
		int cnt = 0;
		
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

	vector<vector<double>> A(info.size(), vector<double>(2));
	vector<double> b_hard(info.size());
	vector<double> b_eng(info.size());

	/*获取A[N*2] b[N*1]的值*/
	int cnt = 0;
	for (auto &ser : info) {
		sServerItem serInfo = ser.second;
		A.at(cnt).at(0) = serInfo.totalCPU;
		A.at(cnt).at(1) = serInfo.totalRAM;
		b_hard.at(cnt) = serInfo.hardCost;
		b_eng.at(cnt) = serInfo.energyCost;
		cnt++;
	}

	/* 计算A^T*A [2*2](转置)*/
	vector<vector<double>> prod{{0,0}, {0,0}};
	for(int iSV = 0; iSV < (int)info.size(); iSV++) {
		prod.at(0).at(0) += A.at(iSV).at(0) * A.at(iSV).at(0);
		prod.at(1).at(1) += A.at(iSV).at(1) * A.at(iSV).at(1);
		prod.at(0).at(1) += A.at(iSV).at(0) * A.at(iSV).at(1);
		prod.at(1).at(0) += A.at(iSV).at(1) * A.at(iSV).at(0);
	}

	/* 计算 (A^T*A)^(-1) [2*2](逆矩阵)*/
	double denom = prod.at(0).at(0) * prod.at(1).at(1) - prod.at(0).at(1) * prod.at(1).at(0); // 分母
	vector<vector<double>> inv(2, vector<double>(2));
	inv.at(0).at(0) = prod.at(1).at(1) / denom;
	inv.at(0).at(1) = -1 * prod.at(0).at(1) / denom;
	inv.at(1).at(0) = -1 * prod.at(1).at(0) / denom;
	inv.at(1).at(1) = prod.at(0).at(0) / denom;

	/*计算A的左伪逆 (A^T*A)^(-1)*A^T [2*N]*/
	vector<vector<double>> pseudoInv(2, vector<double>(info.size()));
	for (int iSV = 0; iSV < (int)info.size(); iSV++) {
		pseudoInv.at(0).at(iSV) = inv.at(0).at(0) * A.at(iSV).at(0) + inv.at(0).at(1) * A.at(iSV).at(1); // 第一行
		pseudoInv.at(1).at(iSV) = inv.at(1).at(0) * A.at(iSV).at(0) + inv.at(1).at(1) * A.at(iSV).at(1); // 第二行
	}

	/*计算方程组的解 (A^T*A)^(-1)*A^T*b */
	double cpuHardVal = 0;
	double ramHardVal = 0;
	double cpuEngVal = 0;
	double ramEngVal = 0;
	for (int iSV = 0; iSV < (int)info.size(); iSV++) {
		cpuHardVal += pseudoInv.at(0).at(iSV) * b_hard.at(iSV);
		ramHardVal += pseudoInv.at(1).at(iSV) * b_hard.at(iSV);
		cpuEngVal += pseudoInv.at(0).at(iSV) * b_eng.at(iSV);
		ramEngVal += pseudoInv.at(1).at(iSV) * b_eng.at(iSV);
	}

	// Return
	RAMHardCost = ramHardVal;
	CPUHardCost = cpuHardVal;
	RAMEnergyCost = ramEngVal;
	CPUEnergyCost = cpuEngVal;

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