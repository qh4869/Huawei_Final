#include "io.h"
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include <iostream>
using namespace std;

// 上传系统要把dataIn函数前三行注释掉
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

    // 输出
    // dataOut()

    return 0;
}

void process(cServer &server, cVM &VM, const cRequests &request) {
	
}