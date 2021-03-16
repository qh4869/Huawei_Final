#include "io.h"
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include "firstFit.h"
#include "graphFit.h"
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
	tie(server, VM, request) = dataIn("training-1.txt");

	// 数据预处理
	server.alpha = ALPHA;
	server.rankServerByPrice(); // 按照价格排序 权重为alpha

	// 购买，迁移，部署
	process(server, VM, request);

#ifndef LOCAL
	// 输出
	dataOut(server, VM, request);
#endif
	//dataOut(server, VM, request);
	//cout << server.price << endl;
	system("pause");
	return 0;
}

void process(cServer &server, cVM &VM, const cRequests &request) {
	//firstFit(server, VM, request);
	graphFit(server, VM, request);
}