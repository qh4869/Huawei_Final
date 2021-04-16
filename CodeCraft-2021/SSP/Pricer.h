#pragma once
#include "SSP_Mig_VM.h"
#include "SSP_Mig_Request.h"
#include "SSP_Mig_Server.h"

class cPricer {
public:
	double genRatio;
	vector<double> fixRate;
	int state = 0;
	bool deter = false; // 标记第1天，第0天进不去调价循环
	bool curstate;
	bool toggle = false;
	double winValue = 0.7;
	double lossValue = 0.3;
	double minRatio = 0.5;
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
	
};