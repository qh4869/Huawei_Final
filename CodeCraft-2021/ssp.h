#pragma once
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include <tuple>
using namespace std;

struct sVmNode {
	string vmID;
	string vmName;
}

void firstFit(cServer &server, cVM &VM, const cRequests &request);