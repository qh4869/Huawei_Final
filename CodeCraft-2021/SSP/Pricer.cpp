#include "Pricer.h"
#include <fstream>

void cPricer::updateRate() {

	/*统计当日的最低折扣*/
	int min;
	auto minIte = min_element(minRatio.begin(), minRatio.end());
	if (minIte == minRatio.end()) // minRatio is empty
		return;
	else
		min = *minIte;

	/*最低折扣不能大于maxRatio*/
	if (min > maxRatio)
		min = maxRatio;

	controlRatio += fixRate.at(state);
	genRatio = min + controlRatio;
	if (genRatio < min) {
		genRatio = min;
		controlRatio = 0;
	}
	else if (genRatio > maxRatio) {
		genRatio = maxRatio;
		controlRatio = maxRatio - min;
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

	// 初始化
	updatePseudoVar(server);
	minRatio.clear();
	double cpuAveRatio, ramAveRatio;
	tie(cpuAveRatio, ramAveRatio) = server.getAveResourceRatio(); // 计算占空比(之前所有天的均值，不考虑空SV)
	double cpuAveRatioIncAll, ramAveRatioIncAll;
	tie(cpuAveRatioIncAll, ramAveRatioIncAll) = server.getAveResourceRatioIncludeALL();

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
			int userTotalQuote = userQuote;
			int estEnergyCost;
			int estHardCost;

			if (vmIsDouble) {
				serID = pseudoFfDouble(vmReqCPU, vmReqRAM);
			}
			else {
				tie(serID, serNode) = pseudoFfSingle(vmReqCPU, vmReqRAM);
			}

			if (serID != -1) { // 装入已有服务器
				if (vmIsDouble)
					pseudoDeploy(VM, vmName, serID);
				else
					pseudoDeploy(VM, vmName, serID, serNode);
				/*每天平摊硬件成本*/ 
				string serName = pseudoServerSet[serID].serName;
				int hardTax, hardBuyTax;
				int purDate;
				if (server.purchaseDate.count(serID)) // 不是当日伪购买的
					purDate = server.purchaseDate[serID];
				else
					purDate = iDay;
				if (purDate != iDay) { // 直接就能放入已有的服务器
					hardBuyTax = 1;
					if (purDate <= earlyDay) { // 前期购入
						hardTax = 1;
					}
					else { // 中期或者后期购入
						hardTax = hardTax0;
					}
				}
				else { // 当天伪购买，同下面伪购买的策略
					if (iDay <= earlyDay) {
						hardTax = 1;
						hardBuyTax = 1;
					}
					else if (iDay <= laterDay) {
						hardTax = hardTax0;
						hardBuyTax = 1;
					}
					else {
						hardTax = hardTax0;
						hardBuyTax = hardTax1;
					}
				}

				if (cpuAveRatioIncAll != -1 && ramAveRatioIncAll != -1) {
					estHardCost = ((double)vmReqCPU * server.CPUHardCost / cpuAveRatioIncAll \
						+  (double)vmReqRAM * server.RAMHardCost / ramAveRatioIncAll) \
						/ (request.dayNum - purDate) * lifeTime * hardTax * hardBuyTax;
				}
				else {
					estHardCost = ((double)vmReqCPU * server.CPUHardCost +  (double)vmReqRAM * server.RAMHardCost) \
						/ (request.dayNum - purDate) * lifeTime * hardTax * hardBuyTax;
				}
				/*平摊功耗成本*/
				if (cpuAveRatio != -1 && ramAveRatio != -1) {
					estEnergyCost = ((double)vmReqCPU * server.CPUEnergyCost / cpuAveRatio \
						+ (double)vmReqRAM * server.RAMEnergyCost / ramAveRatio) \
						* lifeTime;
				}
				else {
					estEnergyCost = ((double)vmReqCPU * server.CPUEnergyCost + (double)vmReqRAM * server.RAMEnergyCost) \
						* lifeTime;
				}
			}
			else {
				string serName = server.chooseSer(vmReqCPU, vmReqRAM, vmIsDouble);
				serID = pseudoPurchase(server, serName);
				if (vmIsDouble)
					pseudoDeploy(VM, vmName, serID);
				else
					pseudoDeploy(VM, vmName, serID, true);
				/*每天平摊硬件成本*/
				int hardTax;
				int hardBuyTax;
				if (iDay <= earlyDay) { // 前期
					hardTax = 1;
					hardBuyTax = 1;
				}
				else if (iDay <= laterDay){ // 中期
					hardTax = hardTax0;
					hardBuyTax = 1;
				}
				else { // 后期
					hardTax = hardTax0;
					hardBuyTax = hardTax1;
				}

				if (cpuAveRatioIncAll != -1 && ramAveRatioIncAll != -1) {
					estHardCost = ((double)vmReqCPU * server.CPUHardCost / cpuAveRatioIncAll \
						+ (double)vmReqRAM * server.RAMHardCost / ramAveRatioIncAll) \
						/ (request.dayNum - iDay) * lifeTime * hardTax * hardBuyTax;
				}
				else {
					estHardCost = ((double)vmReqCPU * server.CPUHardCost + (double)vmReqRAM * server.RAMHardCost) \
						/ (request.dayNum - iDay) * lifeTime * hardTax * hardBuyTax;
				}
				/*平摊功耗成本*/
				if (cpuAveRatio != -1 && ramAveRatio != -1) {
					estEnergyCost = ((double)vmReqCPU * server.CPUEnergyCost / cpuAveRatio \
						+ (double)vmReqRAM * server.RAMEnergyCost / ramAveRatio) \
						* lifeTime;
				}
				else {
					estEnergyCost = ((double)vmReqCPU * server.CPUEnergyCost + (double)vmReqRAM * server.RAMEnergyCost) \
						* lifeTime;
				}			
			}

			minRatio.push_back( (double)(estHardCost + estEnergyCost) / userTotalQuote * estCostScale);
		}
	}

	if (iDay == 0) {
		genRatio = -1; // -1是个标记，即按照minRatio定价
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
	// genRatio = -1;

	/*定价*/
	setEqualPrice(VM, request, iDay);
}

void cPricer::setEqualPrice(cVM &VM, cRequests &request, int iDay) {
/* Fn: 同一天按照同比例出价
*/
	int cnt = 0;

	unordered_map<string, int> dayQuote;  // 当天的报价
	for (auto &req : request.info[iDay]) {
		if (req.type) { // true : add
			if (genRatio == -1) {
				if (minRatio.at(cnt) <= maxRatio)
					dayQuote.insert({req.vmID, req.quote * minRatio.at(cnt)}); // 按照每个请求的成本价出价
				else
					dayQuote.insert({req.vmID, -1});
			}
			else {
				if (genRatio < minRatio.at(cnt) && minRatio.at(cnt) <= maxRatio) // 不能给出低于成本价
					dayQuote.insert({req.vmID, req.quote * minRatio.at(cnt)});
				else if (genRatio < minRatio.at(cnt) && minRatio.at(cnt) > maxRatio) // 不卖
					dayQuote.insert({req.vmID, -1});
				else
					dayQuote.insert({req.vmID, req.quote * genRatio});
			}
			cnt++;
		}
	}
	VM.quote.push_back(dayQuote);
}

void cPricer::updateDayThreadhold(int dayNum) {
	earlyDay = (int)(early * dayNum);
	laterDay = (int)(later * dayNum);
}