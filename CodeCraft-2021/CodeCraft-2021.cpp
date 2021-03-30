#include "io.h"
// #include "SSP_Mig_Server.h"
// #include "SSP_Mig_VM.h"
// #include "ssp.h"
#include "FF_Server.h"
#include "firstFit.h"
#include "Request.h"
#include <iostream>
#include "globalHeader.h" 
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
	fin.open("../CodeCraft-2021/training-2.txt");
	cin.rdbuf(fin.rdbuf());
#endif
	
	cFF_Server server;
	cVM VM;
	cRequests request;

	/*自定义参数*/
	server.alpha = 400;

	/*读取非请求部分*/
	infoDataIn(cin, server, VM, request);

	/*初始化，排序等操作*/
	server.rankServerByPrice();

	for (int iDay = 0; iDay < request.dayNum; iDay++) {
#ifdef LOCAL
		cout << iDay << endl;
		TIMEend = clock();
		cout<<(double)(TIMEend-TIMEstart)/CLOCKS_PER_SEC << 's' << endl;
#endif
		/*读取请求部分*/
		if (!request.readOK)
			reqDataIn(cin, request, iDay);

		/*购买 迁移 部署 删除*/
		firstFitEachDay(iDay, server, VM, request);

		/*id 映射*/
		server.idMappingEachDay(iDay);

#ifndef LOCAL
		/*每日输出*/
		dataOutEachDay(iDay, server, VM, request);
#endif

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
#endif
	
	return 0;
}
