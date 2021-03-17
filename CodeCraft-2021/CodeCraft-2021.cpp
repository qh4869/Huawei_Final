#include "io.h"
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include "ssp.h"
#include <iostream>
#include "globalHeader.h" // 全局常量
using namespace std;

int main()
{
    cServer server;
    cVM VM;
    cRequests request;
    void process(cServer &server, cVM &VM, const cRequests &request);

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

void process(cServer &server, cVM &VM, const cRequests &request) {

    // 数据预处理
    // server.alpha = ALPHA;
    server.ksSize = KSSIZE;
    // server.rankServerByPrice(); // 按照价格排序 权重为alpha

	// firstFit(server, VM, request);
    ssp(server, VM, request);

    server.idMapping(); // id map
}