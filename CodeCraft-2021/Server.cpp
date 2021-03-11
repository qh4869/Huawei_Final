#include "Server.h"

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

	if (buyRecord[iDay].count(name)) 
		buyRecord[iDay][name]++;
	else {
		buyRecord[iDay].insert(std::make_pair(name, 1));
		newServerNum[iDay]++;
	}


}