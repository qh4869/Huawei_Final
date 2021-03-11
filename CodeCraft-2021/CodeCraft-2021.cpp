#include "dataStat.h"
#include <iostream>


// 上传系统要把dataIn函数前三行注释掉
int main()
{
    // TODO:read standard input
    cServer server;
    cVM VM;
    cRequests request;
    tie(server, VM, request) = dataIn("../CodeCraft-2021/training-1.txt");

    // cout << server.serverTypeNum << endl;
    // cout << VM.vmTypeNum << endl;
    // cout << request.info[0][0].vmName << endl;

    // TODO:process
    // TODO:write standard output
    // TODO:fflush(stdout);

    return 0;
}
