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
	ifstream fin;
	fin.open("../CodeCraft-2021/training-1.txt");
	cin.rdbuf(fin.rdbuf());
#endif
	cServer server;
	cVM VM;
	cRequests request;

	/*读取非请求部分*/
	infoDataIn(cin, server, VM, request);

	for (int iDay = 0; iDay < request.dayNum; iDay++) {
#ifdef LOCAL
		cout << iDay << endl;
		TIMEend = clock();
		cout << (double)(TIMEend - TIMEstart) / CLOCKS_PER_SEC << 's' << endl;
#endif
		/*读取请求部分*/
		if (!request.readOK)
			reqDataIn(cin, request, iDay);

		/*购买 迁移 部署 删除*/
		//sspEachDay(iDay, server, VM, request);
		graphFitEachDay(server, VM, request, iDay);

		/*id 映射*/
		server.idMappingEachDay(iDay);

		/*每日输出*/
		dataOutEachDay(iDay, server, VM, request);

#ifdef LOCAL
		/*每日功耗计算*/
		server.energyCostEachDay();
#endif

	}

#ifdef LOCAL
	/*硬件成本计算*/
	server.hardCostTotal();

	/*输出总成本*/
	cout << "成本：" << server.getTotalCost() << endl;

	fin.close();
	//system("pause");
#endif

	return 0;
}

void process(cServer &server, cVM &VM, cRequests &request) {
	// 数据预处理

	double addVar, delVar;
	tie(addVar, delVar) = request.getVarRequest();
	if (delVar < 0.001) {
		ssp(server, VM, request);
	}
	else {
		server.rankServerByPrice(); // 按照价格排序 权重为alpha
		//orderRequest(VM, request);
		graphFit(server, VM, request);
		//request.info = request.realInfo;
	}

	server.idMapping(); // id map

}