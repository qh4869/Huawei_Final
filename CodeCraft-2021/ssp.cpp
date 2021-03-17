#include "ssp.h"

void ssp(cServer &server, cVM &VM, const cRequests &request) {
/* Fn: SSP方法
*	- 关于del的地方还可以优化
*	- 双节点的问题
*	- 背包问题：动态规划，tree分支定界，遗传算法
*	- 有一些vm是可以放进已经买的服务器中的，可以改成dp 或者 排序firstFit
*/
	bool vmIsDouble;
	int serID;
	bool serNode;
	int vmReqCPU; 
	int vmReqRAM;
	int bugID;
	// 记录那些还没被处理的add请求vmID {firstFit放不进去添加/背包部分部署删除}
	unordered_set<string> addSet; 
	unordered_set<string> curSet; // addSet的子集，最多server.ksSize个元素

	for (int iDay=0; iDay<request.dayNum; iDay++) { // 每一天的请求
		/*部分虚拟机装进已购买的服务器，能够处理的del请求（已经部署的vm）也直接处理*/
		for (int iTerm=0; iTerm<request.numEachDay[iDay]; iTerm++) { // 每一条请求
			string vmID = request.info[iDay][iTerm].vmID;

			if (request.info[iDay][iTerm].type) { // add
				/*First fit 没排序版本*/
				string vmName = request.info[iDay][iTerm].vmName;
				vmIsDouble = VM.isDouble(vmName);
				vmReqCPU = VM.reqCPU(vmName);
				vmReqRAM = VM.reqRAM(vmName);

				if (vmIsDouble) {
					serID = server.firstFitDouble(vmReqCPU, vmReqRAM);
				}
				else {
					tie(serID, serNode) = server.firstFitSingle(vmReqCPU, vmReqRAM);
				}

				if (serID != -1) { // 能直接装进去
					if (vmIsDouble) 
						bugID = deploy(server, iDay, vmID, vmName, serID);
					else 
						bugID = deploy(server, iDay, vmID, vmName, serID, serNode);
					if (bugID) {
						cout << "部署失败" << bugID << endl;
						return;
					}
				}
				else // 装不进去
					addSet.insert(vmID);

			}
			else { // del
				/*检查这个vm是否已经被部署 或者 在addSet中*/
				if (VM.workingVmSet.count(vmID))
					VM.deleteVM(vmID, server);
				else if (!sddSet.count(vmID)) {
					cout << "无法删除没有add请求的服务器" << endl;
					return;
				}
			}
		}

		/*迭代把所有add请求的虚拟机，放进服务器，尽量用最少的cost。每次背包问题最多解N台vm，不然复杂度太高*/
		/*由上至下，带记忆的动态规划*/
		//

	}





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

void knapSack(cServer &server, unordered_set<string> curSet) {
	// dp
}

void dp(int )