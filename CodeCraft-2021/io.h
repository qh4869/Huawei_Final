#pragma once
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include <tuple>
#include <fstream>
#include <iostream>
#include "globalHeader.h"
using namespace std;


// �������ݶ�ȡ
std::tuple<cServer, cVM, cRequests> dataIn(string fileName);

//������������׼���
void dataOut(cServer &server, cVM &VM, const cRequests &request);