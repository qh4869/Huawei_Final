#include "FF_Server.h"


bool cFF_Server::mycomp(pair<string, int> i, pair<string, int> j) {
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
