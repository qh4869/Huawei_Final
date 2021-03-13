#include "firstFit.h"

void firstFit(cServer &server, cVM &VM, const cRequests &request) {
/* Fn: first fit算法部署和购买，不迁移
*/
	// 估计成本变量
	int engCostStas = 0; // 计算功耗成本
	int hardCostStas = 0;
	string costName;

	// 初始化变量
	int serId; 
	bool serNode; 
	string serName;
	string vmName;
	string vmID;
	bool vmIsDouble; 
	int vmReqCPU; 
	int vmReqRAM; 

	for (int iDay=0; iDay<request.dayNum; iDay++) {
		for (int iTerm=0; iTerm<request.numEachDay[iDay]; iTerm++) {
			if (request.info[iDay][iTerm].type) { // add 请求
				// 获取虚拟机信息
				vmName = request.info[iDay][iTerm].vmName;
				vmID = request.info[iDay][iTerm].vmID;
				vmIsDouble = VM.isDouble(vmName);
				vmReqCPU = VM.reqCPU(vmName);
				vmReqRAM = VM.reqRAM(vmName);

				// 搜索已有的服务器 从0号服务器开始
				if (vmIsDouble) { // 双节点
					serId = server.firstFitDouble(vmReqCPU, vmReqRAM);
				} 
				else {
					tie(serId, serNode) = server.firstFitSingle(vmReqCPU, vmReqRAM);
				}

				// 根据搜索结果分配或者购买
				if (serId != -1) { // 分配成功
					if (vmIsDouble) 
						VM.deploy(server, iDay, vmID, vmName, serId);
					else
						VM.deploy(server, iDay, vmID, vmName, serId, serNode);
				}
				else { // 需要购买服务器
					serName = server.chooseSer(vmReqCPU, vmReqRAM, vmIsDouble);
					serId = server.purchase(serName, iDay);
					if (vmIsDouble)
						VM.deploy(server, iDay, vmID, vmName, serId);
					else
						VM.deploy(server, iDay, vmID, vmName, serId, true); // 新买的服务器就先放A节点
				}
			}
			else { // delete 请求
				vmID = request.info[iDay][iTerm].vmID;
				VM.deleteVM(vmID, server);
			}
		}
		// 一天结束统计功耗成本
		for (int iServer=0; iServer<server.myServerSet.size(); iServer++) {
			if (server.isOpen(iServer)) {
				costName = server.myServerSet[iServer].serName;
				engCostStas += server.info[costName].energyCost;
			}
		}
	}
	// 计算买服务器总成本
	for (int iServer=0; iServer<server.myServerSet.size(); iServer++) {
		costName = server.myServerSet[iServer].serName;
		hardCostStas += server.info[costName].hardCost;
	}
	cout << "成本: " << engCostStas+hardCostStas << endl;
}