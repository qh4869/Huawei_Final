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
<<<<<<< HEAD
std::tuple<cSspServer, cVM, cRequests> dataIn(string fileName);
=======
std::tuple<cServer, cVM, cRequests> dataIn(string fileName);

//将结果输出到标准输出
void dataOut(cServer &server, cVM &VM, const cRequests &request);
>>>>>>> master
