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