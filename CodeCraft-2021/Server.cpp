#include "Server.h"
#include <iostream>

void cServer::purchase(string name, int iDay) {
	/* Fn:
	*	- name: 服务器型号
	* 	- iDay: 第几天
	*/

	sMyEachServer oneServer;
	oneServer.serName = name;
	oneServer.aIdleCPU = info[name].totalCPU / 2;
	oneServer.bIdleCPU = info[name].totalCPU / 2;
	oneServer.aIdleRAM = info[name].totalRAM / 2;
	oneServer.bIdleRAM = info[name].totalRAM / 2;

	myServerSet.push_back(oneServer);
	dayServerNum[iDay]++;     // 每天服务器加1

	if (buyRecord[iDay].count(name))
		buyRecord[iDay][name]++;
	else {
		buyRecord[iDay].insert(std::make_pair(name, 1));
		newServerNum[iDay]++;
	}

}

//int cServer::purchase(string name, int iDay) {
///* In:
//*	- name: 服务器型号
//* 	- iDay: 第几天
//*	
//* Out:
//*	- 购买的服务器ID
//*/
//
//	sMyEachServer oneServer;
//	oneServer.serName = name;
//	oneServer.aIdleCPU = info[name].totalCPU / 2;
//	oneServer.bIdleCPU = info[name].totalCPU / 2;
//	oneServer.aIdleRAM = info[name].totalRAM / 2;
//	oneServer.bIdleRAM = info[name].totalRAM / 2;
//
//	myServerSet.push_back(oneServer);
//
//	if (buyRecord[iDay].empty() || buyRecord[iDay].back().first!=name) {
//		buyRecord[iDay].push_back(make_pair(name, 1));
//		newServerNum[iDay]++;
//	}
//	else { // 已有相同类型(必然是相连的 ref: cVM:buy())
//		buyRecord[iDay].back().second++;
//	}
//
//	return myServerSet.size()-1;
//
//}

pair<bool, int> cServer::firstFitDouble(int reqCPU, int reqRAM) {
/* Fn: 双节点预部署
* Out: false 表示旧服务器 true表示当天新买的服务器
*/
	for (int iSer=0; iSer<(int)mySerCopy.size(); iSer++) {
		if (mySerCopy[iSer].aIdleCPU > reqCPU/2 && mySerCopy[iSer].bIdleCPU > reqCPU/2 \
			&& mySerCopy[iSer].aIdleRAM > reqRAM/2 && mySerCopy[iSer].bIdleRAM > reqRAM/2)
			return make_pair(false, iSer);
	}
	for (int iSer=0; iSer<(int)preSvSet.size(); iSer++) {
		if (preSvSet[iSer].aIdleCPU > reqCPU/2 && preSvSet[iSer].bIdleCPU > reqCPU/2 \
			&& preSvSet[iSer].aIdleRAM > reqRAM/2 && preSvSet[iSer].bIdleRAM > reqRAM/2)
			return make_pair(true, iSer);
	}

	return make_pair(false, -1);
}

std::tuple<pair<bool, int>, bool> cServer::firstFitSingle(int reqCPU, int reqRAM) {
/* Fn: 单节点部署，bool输入为true表示A节点
*/
	for (int iSer=0; iSer<(int)mySerCopy.size(); iSer++) {
		if (mySerCopy[iSer].aIdleCPU > reqCPU && mySerCopy[iSer].aIdleRAM > reqRAM)
			return make_tuple(make_pair(false, iSer), true);
		if (mySerCopy[iSer].bIdleCPU > reqCPU && mySerCopy[iSer].bIdleRAM > reqRAM)
			return make_tuple(make_pair(false, iSer), false);
	}
	for (int iSer=0; iSer<(int)preSvSet.size(); iSer++) {
		if (preSvSet[iSer].aIdleCPU > reqCPU && preSvSet[iSer].aIdleRAM > reqRAM)
			return make_tuple(make_pair(true, iSer), true);
		if (preSvSet[iSer].bIdleCPU > reqCPU && preSvSet[iSer].bIdleRAM > reqRAM)
			return make_tuple(make_pair(true, iSer), false);
	}

	return make_tuple(make_pair(false, -1), false);
}

bool cServer::mycomp(pair<string, int> i, pair<string, int> j) {
	return i.second < j.second;
}

void cServer::rankServerByPrice() {
	for (auto ite = info.begin(); ite != info.end(); ite++) { // 迭代器
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

	if (myServerSet[serID].aIdleCPU == info[serName].totalCPU / 2 \
		&& myServerSet[serID].bIdleCPU == info[serName].totalCPU / 2 \
		&& myServerSet[serID].aIdleRAM == info[serName].totalRAM / 2 \
		&& myServerSet[serID].bIdleRAM == info[serName].totalRAM / 2)
		return false;
	return true;

}

int cServer::prePurchase(string name) {
	sMyEachServer oneServer;

	oneServer.serName = name;
	oneServer.aIdleCPU = info[name].totalCPU / 2;
	oneServer.bIdleCPU = info[name].totalCPU / 2;
	oneServer.aIdleRAM = info[name].totalRAM / 2;
	oneServer.bIdleRAM = info[name].totalRAM / 2;

	preSvSet.push_back(oneServer);

	return preSvSet.size()-1;

}

void cServer::buy(int iDay) {
/* Fn: preSvSet内容转为myServerSet，同时填写id映射表
*/
	int realid;
	vector<bool> movedFlag(preSvSet.size());
	string curSerName;

	for (int id=0; id<(int)preSvSet.size(); id++) {
		if (movedFlag[id]==true)
			continue;
		//////////////////////////////////////////////////////////////
		//realid = purchase(preSvSet[id].serName, iDay);
		realid = 0; /////////////该函数不可用
		///////////////////////////////////////////////////////
		
		idMap.insert(make_pair(id, realid));
		movedFlag[id] = true;

		curSerName = myServerSet[realid].serName;
		// 再搜索同类型的sv放在一起
		for (int k=id+1; k<(int)preSvSet.size(); k++) {
			if (movedFlag[k]==false && preSvSet[k].serName==curSerName) {
				///////////////////////////////////////////////////////
				//realid = purchase(preSvSet[id].serName, iDay);
				realid = 0;///////////////////////////////////////
				/////////////////////////////////////////
				idMap.insert(make_pair(k, realid));
				movedFlag[k] = true;
			}
		}
	}

	preSvSet.clear();
}


void cServer::rankServerByPrice(bool flag) {

	if (flag) {   // 为true表示第一次生成数据
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

	sort(priceOrder.begin(), priceOrder.end(), cServer::mycomp);

}


/// CYT
// 从小到大排序
bool cServer::rescomp(pair<int, int> i, pair<int, int> j) {
	return i.second < j.second;
}

bool cServer::rescomp_equal(pair<int, int> i, pair<int, int> j) {
	return i.second <= j.second;
}

// 往里增加新服务器时排序
void cServer::rankServerByResource(pair<int, int> newOne) {

	if (resourceOrder.size() == 0) {    // 没有元素则直接加入
		resourceOrder.push_back(newOne);
	}
	else {

		int capacity = resourceOrder.size();
		if (rescomp_equal(resourceOrder[capacity - 1], newOne)) {
			resourceOrder.push_back(newOne);
		}
		else {
			for (int i = 0; i < capacity; i++) {

				if (rescomp_equal(newOne, resourceOrder[i])) {
					resourceOrder.insert(resourceOrder.begin() + i, newOne);
					break;
				}

			}
		}

	}

}

// 每次往服务器添加VM或者删除VM都需要更新服务器的排序
void cServer::updateRank(int buyID, bool flag) { // flag=true表示往里加VM，false表示删除VM

	// 找到对应的位置
	int pos;
	for (int i = 0; i < resourceOrder.size(); i++) {
		if (resourceOrder[i].first == buyID) {
			pos = i;
			break;
		}
	}

	if (flag && pos == 0) {
	}
	else if (!flag && pos == resourceOrder.size() - 1) {
	}
	else {

		pair<int, int> temp = resourceOrder[pos];
		int serID = temp.first;
		auto ite = resourceOrder.begin();
		temp.second = myServerSet[serID].aIdleCPU + myServerSet[serID].bIdleCPU
			+ myServerSet[serID].aIdleRAM + myServerSet[serID].bIdleRAM;

		if (pos - 1 < 0 || rescomp(resourceOrder[pos - 1], temp)) {

			if (pos != resourceOrder.size() - 1) {

				for (int i = pos + 1; i < resourceOrder.size(); i++) {
					if (rescomp_equal(temp, resourceOrder[i])) {
						resourceOrder.erase(ite + pos);
						resourceOrder.insert(resourceOrder.begin() + i - 1, temp);
						break;
					}
				}

			}
			else {
				resourceOrder[pos] = temp;
			}

		}
		else {

			for (int i = pos - 1; i >= 0; i--) {

				if (i == 0 && rescomp_equal(temp, resourceOrder[i])) {
					resourceOrder.erase(ite + pos);
					resourceOrder.insert(resourceOrder.begin() + i, temp);
					break;
				}

				if (rescomp_equal(resourceOrder[i], temp)) {
					resourceOrder.erase(ite + pos);
					resourceOrder.insert(resourceOrder.begin() + i + 1, temp);
					break;
				}
			}

		}

	}

}