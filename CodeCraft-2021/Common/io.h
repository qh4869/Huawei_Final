#pragma once
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include <tuple>
#include <fstream>
#include <iostream>
#include "globalHeader.h"
using namespace std;

// 读取非请求部分
void infoDataIn(istream &cin, cServer &server, cVM &VM, cRequests &request);

// 读取请求部分
void reqDataIn(istream &cin, cRequests &request, int iDay);

// 每天输出
void dataOutEachDay(int iDay, cServer &server, cVM &VM, cRequests &request);

/*********** 决赛新增内容 **********/
// 输出我方报价
void dataOutQuote(int iDay, cVM &VM, cRequests &request);

//void infoDataOut(cServer &server, cVM &VM, ofstream &fout);

void reqDataOut(cRequests &request, ofstream &fout, int iDay);