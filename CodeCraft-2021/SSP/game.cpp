#include "game.h"

void quoteStrategy(int iDay, cSSP_Mig_Request &request, cSSP_Mig_VM &VM, cSSP_Mig_Server &server) {

	unordered_map<int, sMyEachServer> delSerSet; // 有删除操作的SV集合，以及其中扣除del操作的容量
	unordered_set<string> dayWorkingVM;          // 当天新加入的VM集合(就算del也没从中去掉)

	cSSP_Mig_Server rServer = server;       // 保存当天之前的server
	cSSP_Mig_VM rVM = VM;                   // 保存当天之前的VM
	cSSP_Mig_Request rRequest = request;    // 保存当天之前的request

	rServer.ksSize = 1;   // 提一下速度

	// 预先购买和部署
	orderRequest(VM, rRequest, iDay);
	dailyPurchaseDeploy(rServer, rVM, rRequest, iDay, delSerSet, dayWorkingVM);
	rServer.setSourceEmptyRatio(iDay);    // 计算当天的比例

	sSourceRatio sRatio = getSourceEmptyRatio(iDay, server, rServer);

	string vmID, vmName;   // 虚拟机ID和类型
	unordered_map<string, int> dayQuote;    // 当天给的报价
	int hardCost, energyCost;  // 虚拟机的硬件成本和能耗成本
	int cost, uQuote, quote;   // 硬件成本、 用户报价、我方报价
	sVmItem vmInfo;    // 虚拟机信息
	sEachWorkingVM workVM;

	for (auto &req : request.info[iDay]) {
		if (!req.type)      // 删除操作不必理会
			continue;
		vmID = req.vmID;
		vmName = req.vmName;
		uQuote = req.quote;
		vmInfo = VM.info[vmName];
		workVM = rVM.workingVmSet[vmID];
		if (workVM.serverID < (int)server.myServerSet.size()) {    // 表示可以放入当天购买的服务器
			hardCost = ((double)vmInfo.needCPU * server.CPUHardCost / sRatio.CPURatio +    // 加1是因为向下取整了
				(double)vmInfo.needRAM * server.RAMHardCost / sRatio.RAMRatio) / 
				(request.dayNum - server.extraServerInfo[workVM.serverID].buyDay) * req.lifetime + 1;
			energyCost = ((double)vmInfo.needCPU * server.CPUEnergyCost / sRatio.noEmptyCPURatio +  // 加1是因为向下取整了
				(double)vmInfo.needRAM * server.RAMEnergyCost / sRatio.noEmptyRAMRatio) * req.lifetime + 1;
		}
		else {   // 表示放入当天购买的服务器中
			hardCost = ((double)vmInfo.needCPU * server.CPUHardCost / sRatio.CPURatio +    // 加1是因为向下取整了
				(double)vmInfo.needRAM * server.RAMHardCost / sRatio.RAMRatio) /
				(request.dayNum - iDay) * req.lifetime + 1;
			energyCost = ((double)vmInfo.needCPU * server.CPUEnergyCost / sRatio.noEmptyCPURatio +  // 加1是因为向下取整了
				(double)vmInfo.needRAM * server.RAMEnergyCost / sRatio.noEmptyRAMRatio) * req.lifetime + 1;
		}
		cost = int((hardCost + energyCost) * getQuoteRatio(uQuote));   // 不同的定价乘上不同的系数
		VM.extraVmInfo[vmID].cost = cost;    // 保存这台虚拟机的成本价
		quote = getQuote(VM, uQuote, cost, iDay);
		//////////////////////////////////////////////////////////
		if (request.info[iDay].size() > 300 && VM.cytRatio < 0.963) {   // 当天的请求实在太多，并且给的折扣太低
			double tempRatio = rand() % 100 / 101.0;
			if (tempRatio <= 0.5) {
				quote = cost * 0.98;
				if (quote > uQuote)
					quote = -1;
			}
		}
		//////////////////////////////////////////////////////////
		dayQuote.insert({ vmID, quote });
	}
	VM.quote.push_back(dayQuote);
}

sSourceRatio getSourceEmptyRatio(int iDay, cSSP_Mig_Server &server, cSSP_Mig_Server &rServer) {

	// 往后面看
	sSourceRatio forwardRatio;
	forwardRatio.CPURatio = rServer.allSoruceRatio[iDay].first;
	forwardRatio.RAMRatio = rServer.allSoruceRatio[iDay].second;
	forwardRatio.noEmptyCPURatio = rServer.noEmptySourceRatio[iDay].first;;
	forwardRatio.noEmptyRAMRatio = rServer.noEmptySourceRatio[iDay].second;

	// 往前面看
	sSourceRatio backwardRato;
	backwardRato.CPURatio = 0;
	backwardRato.RAMRatio = 0;
	backwardRato.noEmptyCPURatio = 0;
	backwardRato.noEmptyRAMRatio = 0;

	int validCount = 0;
	for (int day = 0; day < iDay; day++) {
		if (server.allSoruceRatio[day].first < 0)
			continue;
		validCount++;
		backwardRato.CPURatio += server.allSoruceRatio[day].first;
		backwardRato.RAMRatio += server.allSoruceRatio[day].second;
		backwardRato.noEmptyCPURatio += server.noEmptySourceRatio[day].first;
		backwardRato.noEmptyRAMRatio += server.noEmptySourceRatio[day].second;
	}
	if (validCount > 0) {
		backwardRato.CPURatio /= iDay;
		backwardRato.RAMRatio /= iDay;
		backwardRato.noEmptyCPURatio /= iDay;
		backwardRato.noEmptyRAMRatio /= iDay;
	}

	// 求出一个平均值
	sSourceRatio sRatio;
	if (iDay == 0 || validCount == 0) {
		sRatio.CPURatio = forwardRatio.CPURatio;
		sRatio.RAMRatio = forwardRatio.RAMRatio;
		sRatio.noEmptyCPURatio = forwardRatio.noEmptyCPURatio;
		sRatio.noEmptyRAMRatio = forwardRatio.noEmptyRAMRatio;
	}
	else {
		double fratio = 0.5, bratio = 0.5;
		sRatio.CPURatio = fratio * forwardRatio.CPURatio + bratio * backwardRato.CPURatio;
		sRatio.RAMRatio = fratio * forwardRatio.RAMRatio + bratio * backwardRato.RAMRatio;
		sRatio.noEmptyCPURatio = fratio * forwardRatio.noEmptyCPURatio + bratio * backwardRato.noEmptyCPURatio;
		sRatio.noEmptyRAMRatio = fratio * forwardRatio.noEmptyRAMRatio + bratio * backwardRato.noEmptyRAMRatio;
	}


	return sRatio;

}

int getQuote(cVM &VM, int uQuote, int cost, int iDay) {

	int quote;
	if (iDay == 0) {    // 第一天先以低价购买
		if (cost > uQuote)
			quote = -1;
		else
			quote = cost;   // 这里可以保证quote不会超过uQuote
		return quote;
	}

	// 第一天之后的定价
	if (cost > uQuote)   // 用户出的钱太低，不要了
		quote = -1;
	else {   // 保证了用户出的钱不会超过uQuote
		quote = int(VM.cytRatio * cost);
		if (quote > uQuote)
			quote = uQuote;
		else if (quote < cost * 0.5)  // 给出的定价太低了
			quote = cost;
	}
	return quote;

}

// 不同报价的虚拟机应该区别对待，对于报价较高的服务器可以适当打折
double getQuoteRatio(int uQuote) {

	if (uQuote <= 5000)   // [0, 5000]
		return 1.02;
	else if (uQuote <= 50000)  // (5000, 50000]
		return 0.98;
	else    // (50000,]
		return 0.95;

}

// 对request按生命周期排序
void orderRequest(cVM &VM, cRequests &request, int iDay) {

	sRequestItem req;
	vector<pair<int, int>> reqOrder;
	vector<sRequestItem> orderInfo;
	for (int iTerm = 0; iTerm < request.info[iDay].size(); iTerm++) {
		req = request.info[iDay][iTerm];
		if (req.type) {    // true : add
			reqOrder.push_back(make_pair(iTerm, req.lifetime));
		}
		else {    // false : delete
			if (!reqOrder.empty()) {    // 非空的话
				sort(reqOrder.begin(), reqOrder.end(), timecomp);
				for (int i = 0; i < (int)reqOrder.size(); i++) {   // 排完序的赋值
					int index = reqOrder[i].first;
					orderInfo.push_back(request.info[iDay][index]);
				}
				reqOrder.clear();
			}
			orderInfo.push_back(request.info[iDay][iTerm]);
		}
	}
	if (!reqOrder.empty()) {
		sort(reqOrder.begin(), reqOrder.end(), timecomp);
		for (int i = 0; i < (int)reqOrder.size(); i++) {   // 排完序的赋值
			int index = reqOrder[i].first;
			orderInfo.push_back(request.info[iDay][index]);
		}
	}
	request.info[iDay] = orderInfo;

}

bool timecomp(pair<int, int> i, pair<int, int> j) {
	return i.second > j.second;
}