#pragma once
#include "VM.h"
#include "Request.h"

class cPricer {
public:
	double genRatio;
	vector<double> upRate;
	vector<double> downRate;
	int state = 0;
	bool deter = false;
	bool curstate;
	bool toggle = false;
	double winValue = 0.7;
	double lossValue = 0.3;
	double minRatio = 0.5;
	double maxRatio = 1;
	int cnt;

	cPricer(double initRatio) : genRatio(initRatio), upRate({0.01, 0.01, 0.0005}), downRate({0.01, 0.01, 0.0005}) {
	}

	void setQuote(cVM &VM, cRequests &request, int iDay);
	void setEqualPrice(cVM &VM, cRequests &request, int iDay);
	void updateRate(int winRatio);
};