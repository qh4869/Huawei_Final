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

void cServer::setSourceEmptyRatio(int iDay) {

	double totalCPU = 0.0, totalRAM = 0.0;
	double noEmptyCPU = 0.0, noEmptyRAM = 0.0;
	double totalVmCPU = 0.0, totalVmRAM = 0.0;

	string serName;
	for (int iSer = 0; iSer < myServerSet.size(); iSer++) {
		serName = myServerSet[iSer].serName;
		totalVmCPU += (info[serName].totalCPU - (myServerSet[iSer].aIdleCPU + myServerSet[iSer].bIdleCPU));
		totalVmRAM += (info[serName].totalRAM - (myServerSet[iSer].aIdleRAM + myServerSet[iSer].bIdleRAM));
		totalCPU += info[serName].totalCPU;
		totalRAM += info[serName].totalRAM;
		if (isOpen(iSer)) {   // 非空服务器
			noEmptyCPU += info[serName].totalCPU;
			noEmptyRAM += info[serName].totalRAM;
		}
	}

	if (myServerSet.size() == 0) {   // 没有买到服务器
		allSoruceRatio.push_back(make_pair(-1, -1));
		noEmptySourceRatio.push_back(make_pair(-1, -1));
	}
	else {
		allSoruceRatio.push_back(make_pair(totalVmCPU / totalCPU, totalVmRAM / totalRAM));
		noEmptySourceRatio.push_back(make_pair(totalVmCPU / noEmptyCPU, totalVmRAM / noEmptyRAM));
	}

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
	vector<vector<double>> prod{ { 0,0 },{ 0,0 } };
	for (int iSV = 0; iSV < (int)info.size(); iSV++) {
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

void cServer::serExtraServerInfo(int iDay) {

	sExtraServerInfo extraInfo;
	int beginSer = extraServerInfo.size();
	for (int iSer = beginSer; iSer < myServerSet.size(); iSer++) { 
		extraInfo.buyDay = iDay;
		extraServerInfo.push_back(extraInfo);
	}

}