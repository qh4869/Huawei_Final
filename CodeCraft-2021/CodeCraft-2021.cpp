#include "io.h"
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include <iostream>
#include "globalHeader.h" // 全局常量
#include "graphFit.h"
#include "ssp.h"
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
	tie(server, VM, request) = dataIn("training-2.txt");

	// 购买，迁移，部署
	//requestOrder(VM, request);
	process(server, VM, request);

#ifndef LOCAL
	// 输出
	dataOut(server, VM, request);
#endif
	//system("pause");
	return 0;
}

void process(cServer &server, cVM &VM, cRequests &request) {
	// 数据预处理

	double addVar, delVar;
	tie(addVar, delVar) = request.getVarRequest();
	if (delVar < 7) {
		ssp(server, VM, request);
	}
	else {
		server.rankServerByPrice(); // 按照价格排序 权重为alpha
		orderRequest(VM, request);
		graphFit(server, VM, request);
		request.info = request.realInfo;
	}

	server.idMapping(); // id map

}