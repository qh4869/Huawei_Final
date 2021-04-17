#include "Pricer.h"
#include <fstream>

void cPricer::updateRate() {

	int min = accumulate(minRatio.begin(), minRatio.end(), 0) / minRatio.size();
	if (min > maxRatio)
		min = maxRatio;

	if (genRatio == -1) {
		genRatio = min;
		return;
	}

	genRatio += fixRate.at(state);
	if (genRatio < min) // genRatio不能偏移的太离谱，最小值为每个请求 成本折扣均值 min(且不能大于1)
		genRatio = min;
	else if (genRatio > maxRatio)
		genRatio = maxRatio;
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
			// userTotalQuote += userQuote;
			int userTotalQuote = userQuote;
			int estEnergyCost;
			int estHardCost;

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
				// sServerItem Serinfo = server.info[serName];
				int hardTax;
				if (iDay <= request.dayNum *4 / 5 && server.purchaseDate[serID] == iDay) // 这个服务器也是今天买的就上税
					hardTax = hardTax0;
				else if (iDay > request.dayNum *4 / 5 && server.purchaseDate[serID] != iDay)
					hardTax = hardTax1;
				else
					hardTax = 1;

				if (cpuAveRatioIncAll != -1 && ramAveRatioIncAll != -1) {
					estHardCost = ((double)vmReqCPU * server.CPUHardCost / cpuAveRatioIncAll \
						+  (double)vmReqRAM * server.RAMHardCost / ramAveRatioIncAll) \
						/ (request.dayNum - purDate) * lifeTime * hardTax;
				}
				else {
					estHardCost = ((double)vmReqCPU * server.CPUHardCost +  (double)vmReqRAM * server.RAMHardCost) \
						/ (request.dayNum - purDate) * lifeTime * hardTax;
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
				// sServerItem Serinfo = server.info[serName];
				int hardTax;
				if (iDay <= request.dayNum *4 / 5)
					hardTax = hardTax0;
				else
					hardTax = hardTax1;

				if (cpuAveRatioIncAll != -1 && ramAveRatioIncAll != -1) {
					estHardCost = ((double)vmReqCPU * server.CPUHardCost / cpuAveRatioIncAll \
						+ (double)vmReqRAM * server.RAMHardCost / ramAveRatioIncAll) \
						/ (request.dayNum - iDay) * lifeTime * hardTax;
				}
				else {
					estHardCost = ((double)vmReqCPU * server.CPUHardCost + (double)vmReqRAM * server.RAMHardCost) \
						/ (request.dayNum - iDay) * lifeTime * hardTax;
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
	// for (auto &x : minRatio) {
	// 	fout << x << endl;
	// }
	// fout << "------------" << endl;
	// 总成本均摊到所有的请求上，再决定最小的折扣
	// if (userTotalQuote != 0) // 万一某一天没有add请求，除数就是0了
	// 	minRatio = (double)(estHardCost + estEnergyCost) / userTotalQuote * 1.5;

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
	genRatio = -1;

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