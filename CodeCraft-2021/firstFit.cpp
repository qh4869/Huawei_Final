#include "firstFit.h"

void firstFit(cServer &server, cVM &VM, const cRequests &request) {
/* Fn: first fit算法部署和购买，不迁移
*/
#ifdef LOCAL
	// 估计成本变量
	int engCostStas = 0; // 计算功耗成本
	int hardCostStas = 0;
	string costName;
#endif

	// 初始化变量
	int serId; 
	pair<bool, int> serPosId;
	bool serNode; 
	string serName;
	string vmName;
	string vmID;
	bool vmIsDouble; 
	int vmReqCPU; 
	int vmReqRAM; 
	int bugID;

	for (int iDay=0; iDay<request.dayNum; iDay++) {
		for (int iTerm=0; iTerm<request.numEachDay[iDay]; iTerm++) {
			// 预购买和预部署
			if (request.info[iDay][iTerm].type) { // add 请求
				// 获取虚拟机信息
				vmName = request.info[iDay][iTerm].vmName;
				vmID = request.info[iDay][iTerm].vmID;
				vmIsDouble = VM.isDouble(vmName);
				vmReqCPU = VM.reqCPU(vmName);
				vmReqRAM = VM.reqRAM(vmName);

				// 搜索已有的服务器 从0号服务器开始
				if (vmIsDouble) { // 双节点
					serPosId = server.firstFitDouble(vmReqCPU, vmReqRAM);
				}
				else {
					tie(serPosId, serNode) = server.firstFitSingle(vmReqCPU, vmReqRAM);
				}

				// 根据搜索结果预购买或预部署
				if (serPosId.second != -1) { // 不用购买，但也有可能是今天买的
					if (vmIsDouble) 
						VM.predp(vmID, serPosId.first, serPosId.second, vmName, server);
					else
						VM.predp(vmID, serPosId.first, serPosId.second, vmName, server, serNode);
				}
				else { // 需要购买
					serName = server.chooseSer(vmReqCPU, vmReqRAM, vmIsDouble);
					serId = server.prePurchase(serName);
					if (vmIsDouble)
						VM.predp(vmID, true, serId, vmName, server);
					else
						VM.predp(vmID, true, serId, vmName, server, true);
				}			
			}
			else { // delete 请求
				vmID = request.info[iDay][iTerm].vmID;
				VM.preDltVM(server, vmID);
			}
		}
		// 正式购买
		server.buy(iDay);
		// 按顺序处理请求
		bugID = VM.postDpWithDel(server, request, iDay);
		if (bugID) {
			cout << "部署失败" << bugID << endl;
			return;
		}
#ifdef LOCAL
		// 一天结束统计功耗成本
		for (int iServer=0; iServer<(int)server.myServerSet.size(); iServer++) {
			if (server.isOpen(iServer)) {
				costName = server.myServerSet[iServer].serName;
				engCostStas += server.info[costName].energyCost;
			}
		}
#endif
	}
#ifdef LOCAL
	// 计算买服务器总成本
	for (int iServer=0; iServer<(int)server.myServerSet.size(); iServer++) {
		costName = server.myServerSet[iServer].serName;
		hardCostStas += server.info[costName].hardCost;
	}
	cout << "成本: " << engCostStas+hardCostStas << endl;
#endif
}