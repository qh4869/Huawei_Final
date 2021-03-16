#pragma once
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include <iostream>
#include "globalHeader.h"
using namespace std;

void firstFit(cServer &server, cVM &VM, const cRequests &request);