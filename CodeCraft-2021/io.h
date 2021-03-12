#pragma once
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include <tuple>
#include <fstream>
using namespace std;


// 输入数据读取
std::tuple<cSspServer, cVM, cRequests> dataIn(string fileName);