#include "FF_Server.h"

int cFF_Server::firstFitDouble(int reqCPU, int reqRAM) {
/* Fn: 双节点部署
*/
	for (int iSer = 0; iSer<(int)myServerSet.size(); iSer++) {
		if (myServerSet[iSer].aIdleCPU > reqCPU / 2 && myServerSet[iSer].bIdleCPU > reqCPU / 2 \
			&& myServerSet[iSer].aIdleRAM > reqRAM / 2 && myServerSet[iSer].bIdleRAM > reqRAM / 2)
			return iSer;
	}

	return -1;
}

std::tuple<int, bool> cFF_Server::firstFitSingle(int reqCPU, int reqRAM) {
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

int cFF_Server::firstFitByIdleOrdDouble(int reqCPU, int reqRAM) {
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

std::tuple<int, bool> cFF_Server::firstFitByIdleOrdSingle(int reqCPU, int reqRAM) {
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


bool cFF_Server::mycomp(pair<string, int> i, pair<string, int> j) {
	return i.second < j.second;
}

bool cFF_Server::mycompID(pair<int, int> i, pair<int, int> j) {
	return i.second < j.second;
}

bool cFF_Server::mycompIDDB(pair<int, double> i, pair<int, double> j) {
	return i.second < j.second;
}

void cFF_Server::rankServerByPrice() {
	/* Fn: 所有服务器类型的排序
	*/
	priceOrder.clear();
	for (auto ite = info.begin(); ite != info.end(); ite++) { // 迭代器
		priceOrder.push_back(make_pair(ite->first, ite->second.hardCost + ite->second.energyCost * alpha));
	}

	sort(priceOrder.begin(), priceOrder.end(), cFF_Server::mycomp);
}

string cFF_Server::chooseSer(int reqCPU, int reqRAM, bool isDoubleNode) {
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

void cFF_Server::rankMyServerbyIdle() {
	/* Fn: 按照空闲资源的顺序对已有的服务器排序，结果装入myServerOrder <vmName, idle resource>
	*	- 四种资源直接求和得到结果
	*/
	myServerOrder.clear();
	for (int iSer = 0; iSer<(int)myServerSet.size(); iSer++) {
		myServerOrder.push_back(make_pair(iSer, myServerSet[iSer].aIdleCPU + myServerSet[iSer].aIdleRAM \
			+ myServerSet[iSer].bIdleCPU + myServerSet[iSer].bIdleRAM));
	}

	sort(myServerOrder.begin(), myServerOrder.end(), cFF_Server::mycompID);
}

void cFF_Server::rankMyServerbyOccupied() {
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

	sort(myServerOrder.begin(), myServerOrder.end(), cFF_Server::mycompID);
}

void cFF_Server::rankMyserverbyRatio() {
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

	sort(myServerOrderDB.begin(), myServerOrderDB.end(), cFF_Server::mycompIDDB);
}