#include "dataStat.h"
using namespace std;

std::tuple<cServer, cVM, cRequests> dataIn(string fileName) {
/*
* Fn: 读入数据
*
* In:
*   - fileName: txt文件名
*   
*/

    ifstream fin; 
    fin.open(fileName.c_str());
    cin.rdbuf(fin.rdbuf()); // 重定向

    string inputStr;
    cServer server;
    cVM VM;
    cRequests request;

    // 服务器种类数
    cin >> inputStr;
    server.serverTypeNum = atoi(inputStr.c_str()); // string to int
    // 服务器每条信息
    server.info.resize(server.serverTypeNum);
    for (int iTerm=0; iTerm<server.serverTypeNum; iTerm++) { 
        // server name
        cin >> inputStr;
        inputStr.erase(inputStr.begin()); // 删除左括号
        inputStr.erase(inputStr.end()-1); // 逗号
        server.info[iTerm].serName = inputStr;
        // cpu
        cin >> inputStr;
        inputStr.erase(inputStr.end()-1);
        server.info[iTerm].totalCPU = atoi(inputStr.c_str());
        // RAM
        cin >> inputStr;
        inputStr.erase(inputStr.end()-1);
        server.info[iTerm].totalRAM = atoi(inputStr.c_str());
        // hardware price
        cin >> inputStr;
        inputStr.erase(inputStr.end()-1);
        server.info[iTerm].hardCost = atoi(inputStr.c_str());
        // energy price
        cin >> inputStr;
        inputStr.erase(inputStr.end()-1);
        server.info[iTerm].energyCost = atoi(inputStr.c_str());
    }

    // 虚拟机总数
    cin >> inputStr;
    VM.vmTypeNum = atoi(inputStr.c_str());
    // 虚拟机的每条信息
    for (int iTerm=0; iTerm<VM.vmTypeNum; iTerm++) {
        // vm name
        string vmName;
        cin >> inputStr;
        inputStr.erase(inputStr.begin()); // 删除左括号
        inputStr.erase(inputStr.end()-1); // 逗号
        vmName = inputStr;

        // vm CPU
        cin >> inputStr;
        inputStr.erase(inputStr.end()-1);
        VM.info[vmName].needCPU = atoi(inputStr.c_str());

        // vm RAM
        cin >> inputStr;
        inputStr.erase(inputStr.end()-1);
        VM.info[vmName].needRAM = atoi(inputStr.c_str());

        // double Node
        cin >> inputStr;
        inputStr.erase(inputStr.end()-1);
        VM.info[vmName].nodeStatus = (inputStr == "1") ? true : false;
    }

    // 请求数据天数
    cin >> inputStr;
    request.dayNum = atoi(inputStr.c_str());
    request.numEachDay.resize(request.dayNum);
    request.info.resize(request.dayNum);
    // 每天的请求
    for (int iDay=0; iDay<request.dayNum; iDay++) {
        // 每天的请求数目
        cin >> inputStr;
        request.numEachDay[iDay] = atoi(inputStr.c_str());
        request.info[iDay].resize(request.numEachDay[iDay]);

        // 每条的请求信息
        for (int iTerm=0; iTerm<request.numEachDay[iDay]; iTerm++) {
            // 请求类型
            cin >> inputStr;
            inputStr.erase(inputStr.begin()); // 删除左括号
            inputStr.erase(inputStr.end()-1); // 逗号
            request.info[iDay][iTerm].type = (inputStr=="add")? true : false;

            // 请求虚拟机的型号
            if (request.info[iDay][iTerm].type) { // add输入虚拟机型号，del不输入型号
                cin >> inputStr;
                inputStr.erase(inputStr.end()-1);
                request.info[iDay][iTerm].vmName = inputStr;
            }

            // 请求虚拟机ID
            cin >> inputStr;
            inputStr.erase(inputStr.end()-1);
            request.info[iDay][iTerm].vmID = inputStr;
        }
    }

    // cout << server.serverTypeNum << endl;

    return make_tuple(server, VM, request);
}