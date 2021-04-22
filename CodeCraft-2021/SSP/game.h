#pragma once
#include "SSP_Mig_Server.h"
#include "SSP_Mig_VM.h"
#include "SSP_Mig_Request.h"
#include "MigrationPlus.h"
#include <fstream>
#include <iostream>
using namespace std;

struct sSourceRatio {
	double CPURatio;
	double RAMRatio;
	double noEmptyCPURatio;
	double noEmptyRAMRatio;
};

void quoteStrategy(int iDay, cSSP_Mig_Request &request, cSSP_Mig_VM &VM, cSSP_Mig_Server &server);

sSourceRatio getSourceEmptyRatio(int iDay, cSSP_Mig_Server &server, cSSP_Mig_Server &rServer);

int getQuote(cVM &VM, int uQuote, int cost, int iDay);

double getQuoteRatio(int uQuote);

void orderRequest(cVM &VM, cRequests &request, int iDay);

static bool timecomp(pair<int, int> i, pair<int, int> j);