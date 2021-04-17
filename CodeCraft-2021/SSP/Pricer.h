#pragma once
#include "SSP_Mig_VM.h"
#include "SSP_Mig_Request.h"
#include "SSP_Mig_Server.h"
#include <numeric>

class cPricer {
public:
	double genRatio; // <第一天估计成本后要初始化，之后每次调价更新>
	vector<double> fixRate;
	int state = 0;
	bool deter = false; // 标记第1天，第0天进不去调价循环
	bool curstate;
	bool toggle = false;
	double winValue = 0.7;
	double lossValue = 0.3;
	// double minRatio = 0.5;
	vector<double> minRatio; // 对应于每个add请求的成本折扣 <定价估计成本时初始化>
	double maxRatio = 1;
	int cnt;

	cPricer() : genRatio(-1), fixRate({0.01, -0.01, 0.0005}) {
	}

	void setQuote(cVM &VM, cRequests &request, cSSP_Mig_Server &server, int iDay);
	void setEqualPrice(cVM &VM, cRequests &request, int iDay);
	void updateRate();

	/*伪部署*/
	vector<sMyEachServer> pseudoServerSet;
	void updatePseudoVar(cServer &server);
	int pseudoFfDouble(int reqCPU, int reqRAM);
	std::tuple<int, bool> pseudoFfSingle(int reqCPU, int reqRAM);
	void pseudoDeploy(cVM &VM, string vmName, int serID);
	void pseudoDeploy(cVM &VM, string vmName, int serID, bool serNode);
	int pseudoPurchase(cServer &server, string serName);

	/*自定义参数*/
	double estCostScale = 1.4;
	double hardTax0 = 1; // 需要购买服务器的请求 更多提价，因为这个SV空闲天数 是没有均摊价格的VM
	double hardTax1 = 1; // 后20%天数，购买服务器的请求涨价
	
};