#include "ssp.h"

void ssp(cServer &server, cVM &VM, const cRequests &request) {
/* Fn: SSP方法
*	- 关于del的地方还可以优化
*	- 双节点的问题
*	- 背包问题：动态规划，tree分支定界
*	- 有一些vm是可以放进已经买的服务器中的，可以改成dp
*/
	vector<pair<string, string>> addList; // <vmID, vmName> 初始化->预部署之后删除元素
	// vector<pair<string, string>> delList;
	// int addNum = 0;
	vector<vector<int>> mat_;
	vector<vector<int>> mat;
	vector<vector<vector<string>>>; // trace path
	string vmName;
	bool vmIsDouble;
	pair<bool, int> serPosId;
	int vmReqCPU; 
	int vmReqRAM;
	bool serNode; 
	string vmID;

	for (int iDay=0; iDay<request.dayNum; iDay++) {
		// 当天的所有请求预处理
		for (int iTerm=0; iTerm<request.numEachDay[iDay]; iTerm++) {
			if (request.info[iDay][iTerm].type) { // add
				addList.push_back(make_pair(request.info[iDay][iTerm].vmID, \
					request.info[iDay][iTerm].vmName));
			}
			else {
				// delList.push_back(make_pair(request.info[iDay][iTerm].vmID, \
				// 	request.info[iDay][iTerm].vmName));
			}
		}
		// addNum = addList.size();

		// 装进已有的服务器 FF
		for (auto it=addList.begin(); it!=addList.end(); it++) {
			vmID = it->first;
			vmName = it->second;
			vmIsDouble = VM.isDouble(vmName);
			vmReqCPU = VM.reqCPU(vmName);
			vmReqRAM = VM.reqRAM(vmName);

			if (vmIsDouble) {
				serPosId = server.firstFitDouble(vmReqCPU, vmReqRAM);
			}
			else {
				tie(serPosId, serNode) = server.firstFitSingle(vmReqCPU, vmReqRAM);
			}

			if (serPosId.second != -1) { // 能直接装进去
				if (serPosId.first==true) {
					cout << "预购买服务器此时不允许装载虚拟机" << endl;
					return;
				}

				if (vmIsDouble) {
					VM.predp(vmID, serPosId.first, serPosId.second, vmName, server);
				}
				else {
					VM.predp(vmID, serPosId.first, serPosId.second, vmName, server, serNode);
				}

				addList.erase(it);
			}

			// ssp
			while (!addList.empty()) {
				// 遍历所有可用服务器
				for (auto it=server.info.begin(); it!=server.info.end(); it++) { // 迭代哈希表
					// 已经拿出一台服务器，开始解背包问题
					// 由下至上 迭代删除记录 解追踪的 动态规划 优化目标为加权资源和
					mat_.clear();
					mat_.resize(server.info[it->second.totalCPU]);
					for (auto &x : mat_)
						x.resize(server.info[it->second.totalRAM]);
					mat.resize()
					mat.clear();
					mat.resize(server.info[it->second.totalCPU]);
					for (auto &x : mat)
						x.resize(server.info[it->second.totalRAM]);
				}
			}


		}

	}
}