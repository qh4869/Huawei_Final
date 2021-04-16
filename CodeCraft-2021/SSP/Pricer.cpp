#include "Pricer.h"
#include <fstream>

void cPricer::updateRate() {
	if (minRatio >= 1) {
		genRatio = -1; // -1折表示不卖
		return;
	}

	if (genRatio == -1) // 上一天没有卖，这一天就最低价开始
		genRatio = minRatio;
	else {
		genRatio += fixRate.at(state);
		if (genRatio < minRatio)
			genRatio = minRatio;
		else if (genRatio > maxRatio)
			genRatio = maxRatio;
	}
}

void cPricer::updatePseudoVar(cServer &server) {
	pseudoServerSet = server.myServerSet;
}

int cPricer::pseudoFfDouble(int reqCPU, int reqRAM) {
/* Fn: 双节点搜索已有服务器-pseudo
*/
	for (int iSer = 0; iSer < (int)pseudoServerSet.size(); iSer++) {
		if (pseudoServerSet[iSer].aIdleCPU >= reqCPU / 2 && pseudoServerSet[iSer].aIdleRAM >= reqRAM / 2 \
			&& pseudoServerSet[iSer].bIdleCPU >= reqCPU / 2 && pseudoServerSet[iSer].bIdleRAM >= reqRAM / 2)
			return iSer;
	}

	return -1;
}

std::tuple<int, bool> cPricer::pseudoFfSingle(int reqCPU, int reqRAM) {
/* Fn: 单节点搜索已有服务器-pseudo
*/
	for (int iSer = 0; iSer < (int)pseudoServerSet.size(); iSer++) {
		if (pseudoServerSet[iSer].aIdleCPU >= reqCPU && pseudoServerSet[iSer].aIdleRAM >= reqRAM)
			return {iSer, true};
		if (pseudoServerSet[iSer].bIdleCPU >= reqCPU && pseudoServerSet[iSer].bIdleRAM >= reqRAM)
			return {iSer, false};
	}

	return {-1, false};
}

void cPricer::pseudoDeploy(cVM &VM, string vmName, int serID) {
/* Fn: 双节点伪部署
*/
	int needCPU = VM.reqCPU(vmName);
	int needRAM = VM.reqRAM(vmName);

	pseudoServerSet[serID].aIdleCPU -= needCPU / 2;
	pseudoServerSet[serID].aIdleRAM -= needRAM / 2;
	pseudoServerSet[serID].bIdleCPU -= needCPU / 2;
	pseudoServerSet[serID].bIdleRAM -= needRAM / 2;
}

void cPricer::pseudoDeploy(cVM &VM, string vmName, int serID, bool serNode) {
/* Fn: 单节点伪部署
*/
	int needCPU = VM.reqCPU(vmName);
	int needRAM = VM.reqRAM(vmName);

	if (serNode) {
		pseudoServerSet[serID].aIdleCPU -= needCPU;
		pseudoServerSet[serID].aIdleRAM -= needRAM;
	}
	else {
		pseudoServerSet[serID].bIdleCPU -= needCPU;
		pseudoServerSet[serID].bIdleRAM -= needRAM;
	}
}

int cPricer::pseudoPurchase(cServer &server, string serName) {
/* Fn: 伪购买
*/
	sMyEachServer oneServer;

	oneServer.serName = serName;
	oneServer.aIdleCPU = server.info[serName].totalCPU / 2;
	oneServer.aIdleRAM = server.info[serName].totalRAM / 2;
	oneServer.bIdleCPU = server.info[serName].totalCPU / 2;
	oneServer.bIdleRAM = server.info[serName].totalRAM / 2;

	pseudoServerSet.push_back(oneServer);

	return pseudoServerSet.size() - 1;
}

void cPricer::setQuote(cVM &VM, cRequests &request, cSSP_Mig_Server &server, int iDay) {
/* Fn: 有限状态机方案
*/

	// ofstream fout;
	// fout.open("tmpOut.txt", ios::app);

	// 更新伪变量
	updatePseudoVar(server);

	/*FF部署，估计成本*/
	int estHardCost = 0;
	int estEnergyCost = 0;
	int userTotalQuote = 0;
	for (int iTerm=0; iTerm<request.numEachDay[iDay]; iTerm++) {
		string vmID = request.info[iDay][iTerm].vmID;
		if (request.info[iDay][iTerm].type) { // add
			string vmName = request.info[iDay][iTerm].vmName;
			bool vmIsDouble = VM.isDouble(vmName);
			int vmReqCPU = VM.reqCPU(vmName);
			int vmReqRAM = VM.reqRAM(vmName);
			int lifeTime = request.info[iDay][iTerm].lifetime;
			int userQuote = request.info[iDay][iTerm].quote;
			int serID;
			bool serNode;

			/*所有请求的用户出价求和*/
			userTotalQuote += userQuote;

			if (vmIsDouble) {
				serID = pseudoFfDouble(vmReqCPU, vmReqRAM);
			}
			else {
				tie(serID, serNode) = pseudoFfSingle(vmReqCPU, vmReqRAM);
			}

			if (serID != -1) {
				if (vmIsDouble)
					pseudoDeploy(VM, vmName, serID);
				else
					pseudoDeploy(VM, vmName, serID, serNode);
				/*每天平摊硬件成本*/ // 这种方法估计的成本是比较保守的，因为服务器会装不满
				int purDate = server.purchaseDate[serID];
				string serName = pseudoServerSet[serID].serName;
				sServerItem Serinfo = server.info[serName];
				estHardCost += Serinfo.hardCost / (request.dayNum - purDate) * lifeTime \
					* (vmReqCPU + vmReqRAM) / (Serinfo.totalCPU + Serinfo.totalRAM);
				/*平摊功耗成本*/
				estEnergyCost += Serinfo.energyCost * lifeTime \
					* (vmReqCPU + vmReqRAM) / (Serinfo.totalCPU + Serinfo.totalRAM);
			}
			else {
				string serName = server.chooseSer(vmReqCPU, vmReqRAM, vmIsDouble);
				serID = pseudoPurchase(server, serName);
				if (vmIsDouble)
					pseudoDeploy(VM, vmName, serID);
				else
					pseudoDeploy(VM, vmName, serID, true);
				/*每天平摊硬件成本*/
				sServerItem Serinfo = server.info[serName];
				estHardCost += Serinfo.hardCost / (request.dayNum - iDay) * lifeTime \
					* (vmReqCPU + vmReqRAM) / (Serinfo.totalCPU + Serinfo.totalRAM);
				/*平摊功耗成本*/
				estEnergyCost += Serinfo.energyCost * lifeTime \
					* (vmReqCPU + vmReqRAM) / (Serinfo.totalCPU + Serinfo.totalRAM);
			}
		}
	}
	// 总成本均摊到所有的请求上，再决定最小的折扣
	if (userTotalQuote != 0) // 万一某一天没有add请求，除数就是0了
		minRatio = (double)(estHardCost + estEnergyCost) / userTotalQuote * 1.5;

	if (iDay == 0) {
		genRatio = minRatio; // 第一天直接给成本价
	}
	else { // 之后的每一天要调价
		// 前一天的胜率
		double winRatio = VM.winNum.at(iDay-1) / (VM.winNum.at(iDay-1) + VM.lossNum.at(iDay-1));

		/*查看win/loss反转，如果winRatio在lossValue和winValue之间，那就不可能win/loss反转*/
		if (winRatio < lossValue) {
			if (!deter) { // 第1天
				state = 1;
				curstate = true;
				deter = true;
			}
			else if (curstate == false){
				curstate = true;
				toggle = true;
			}
		}
		else if (winRatio > winValue) {
			if (!deter) {
				state = 0;
				curstate = false;
				deter = true;
			}
			else if (curstate == true) {
				curstate = false;
				toggle = true;
			}
		}

		/*state2的计数器*/
		if (state == 2)
			cnt++;

		/*状态转移*/
		switch (state) {
			case 0: // 涨价状态
				if (toggle) {
					toggle = false;
					state = 1;	
				}
				break;
			case 1: // 降价状态
				if (toggle) {
					toggle = false;
					state = 2;
					cnt = 0;
				}
				break;
			case 2: // 维持稳定，缓慢上涨
				if (toggle) {
					toggle = false;
					state = 1;
				}
				else if (cnt >= 50) { // 50次计数
					state = 0;
				}
				break;
		}

		/*调价*/
		updateRate();
	}

	/*定价*/
	setEqualPrice(VM, request, iDay);
}

void cPricer::setEqualPrice(cVM &VM, cRequests &request, int iDay) {
/* Fn: 同一天按照同比例出价
*/
	unordered_map<string, int> dayQuote;  // 当天的报价
	for (auto &req : request.info[iDay]) {
		if (req.type) { // true : add
			if (genRatio != -1)
				dayQuote.insert({ req.vmID, req.quote * genRatio}); 
			else // 不卖
				dayQuote.insert({req.vmID, -1});
		} 
	}
	VM.quote.push_back(dayQuote);
}