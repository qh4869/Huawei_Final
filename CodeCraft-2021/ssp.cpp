#include "ssp.h"

void ssp(cServer &server, cVM &VM, const cRequests &request) {
/* Fn: SSP方法
*	- 关于del的地方还可以优化
*	- 双节点的问题
*	- 背包问题：动态规划，tree分支定界
*/
	vector<pair<string, string>> addList; // <vmID, vmName>
	vector<pair<string, string>> delList;
	int addNum = 0;
	// 动态规划的数据结构

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

		// 二分+剪枝（简化版动态规划）

	}
}