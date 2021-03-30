#pragma once
#include "FF_Server.h"
#include "VM.h"
#include "Request.h"
#include <iostream>
#include "globalHeader.h"
#include <time.h>
using namespace std;

void firstFit(cFF_Server &server, cVM &VM, const cRequests &request);

void firstFitEachDay(int iDay, cFF_Server &server, cVM &VM, cRequests &request);

#ifdef LOCAL
extern clock_t TIMEstart, TIMEend;
#endif