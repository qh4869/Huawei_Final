#pragma once
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include <iostream>
#include "globalHeader.h"
#include <time.h>
using namespace std;

void firstFit(cServer &server, cVM &VM, const cRequests &request);

#ifdef LOCAL
extern clock_t TIMEstart, TIMEend;
#endif