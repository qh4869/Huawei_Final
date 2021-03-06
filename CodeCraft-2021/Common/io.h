#pragma once
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include <tuple>
#include <fstream>
#include <iostream>
#include "globalHeader.h"
using namespace std;


// 输入数据读取
void dataIn(string fileName, cServer &server, cVM &VM, cRequests &request);

//将结果输出到标准输出
void dataOut(cServer &server, cVM &VM, const cRequests &request);

// 读取非请求部分
void infoDataIn(istream &cin, cServer &server, cVM &VM, cRequests &request);

// 读取请求部分
void reqDataIn(istream &cin, cRequests &request, int iDay);

// 每天输出
void dataOutEachDay(int iDay, cServer &server, cVM &VM, cRequests &request);
