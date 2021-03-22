#include "io.h"
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include <iostream>
#include "globalHeader.h" // 全局常量
#include "graphFit.h"
using namespace std;

int main()
{
	cServer server;
	cVM VM;
	cRequests request;
	void process(cServer &server, cVM &VM, cRequests &request);

	// 输入
	tie(server, VM, request) = dataIn("training-2.txt");

	// 购买，迁移，部署
	//requestOrder(VM, request);
	process(server, VM, request);

#ifndef LOCAL
	// 输出
	dataOut(server, VM, request);
#endif
	//dataOut(server, VM, request);
	system("pause");
	return 0;
}

void process(cServer &server, cVM &VM, cRequests &request) {
	// 数据预处理
	server.alpha = ALPHA;
	server.rankServerByPrice(); // 按照价格排序 权重为alpha

	graphFit(server, VM, request);

	server.idMapping(); // id map
}