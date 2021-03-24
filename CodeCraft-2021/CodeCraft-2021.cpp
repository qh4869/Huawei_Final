#include "io.h"
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include "ssp.h"
#include <iostream>
#include "globalHeader.h" // 全局常量
#ifdef LOCAL
#include<time.h>
#endif

using namespace std;

#ifdef LOCAL
    clock_t TIMEstart, TIMEend;
#endif
int main()
{
#ifdef LOCAL
    TIMEstart = clock();
#endif
    cServer server;
    cVM VM;
    cRequests request;
    void process(cServer &server, cVM &VM, cRequests &request);

    // 输入
    tie(server, VM, request) = dataIn("../CodeCraft-2021/training-1.txt");

    // 购买，迁移，部署
    process(server, VM, request);

#ifndef LOCAL
    // 输出
    dataOut(server, VM, request);
#endif

    return 0;
}

void process(cServer &server, cVM &VM, cRequests &request) {

    ssp(server, VM, request);
    
    server.idMapping(); // id map

}