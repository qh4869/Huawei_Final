#include "ssp.h"

void ssp(cServer &server, cVM &VM, const cRequests &request) {
/* Fn: SSP方法
*	- 关于del的地方还可以优化
*	- 双节点的问题
*	- 背包问题：动态规划，tree分支定界
*	- 有一些vm是可以放进已经买的服务器中的，可以改成dp
*/
	vector<pair<string, string>> addList; // <vmID, vmName>
	vector<pair<string, string>> delList;
	int addNum = 0;
	vector<vector<vector<int>>> dynaMat; // 不知道内存够不够

	for (int iDay=0; iDay<request.dayNum; iDay++) {
		// 当天的所有请求预处理
		for (int iTerm=0; iTerm<request.numEachDay[iDay]; iTerm++) {
			if (request.info[iDay][iTerm].type) { // add
				addList.push_back(make_pair(request.info[iDay][iTerm].vmID, \
					request.info[iDay][iTerm].vmName));
			}
			else {
				delList.push_back(make_pair(request.info[iDay][iTerm].vmID, \
					request.info[iDay][iTerm].vmName));
			}
		}
		addNum = addList.size();

		// 装进已有的服务器 FF
		

	}
}