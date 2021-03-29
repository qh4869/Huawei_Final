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
