#include "io.h"
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include "firstFit.h"
#include <iostream>
using namespace std;

const int ALPHA = 400; // 价格加权参数

// 上传系统要把dataIn函数前三行注释掉，process中估计成本部分也得去掉
int main()
{
    cServer server;
    cVM VM;
    cRequests request;
    void process(cServer &server, cVM &VM, const cRequests &request);

    // 输入
    tie(server, VM, request) = dataIn("../CodeCraft-2021/training-1.txt");

    // 数据预处理
    server.alpha = ALPHA;
    server.rankServerByPrice(); // 按照价格排序 权重为alpha

    // 购买，迁移，部署
    process(server, VM, request);

    // 输出
    // dataOut()

    return 0;
}

void process(cServer &server, cVM &VM, const cRequests &request) {
	firstFit(server, VM, request);
}