﻿#include "io.h"
#include "SSP_Mig_Server.h"
#include "SSP_Mig_VM.h"
#include "SSP_Mig_Request.h"
#include "ssp.h"
#include "game.h"
// #include "FF_Server.h"
// #include "firstFit.h"
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
	fin.open("game.txt");
	cin.rdbuf(fin.rdbuf());
#endif
	
	cSSP_Mig_Server server;
	cSSP_Mig_VM VM;
	cSSP_Mig_Request request;

	/*自定义参数*/ // 加不加锁
	server.ksSize = 4;
	server.gBeta = 0.49;
	server.gGamma = 1.03;
	server.args[2] = 0.51;
	server.args[3] = 1.02;
	VM.migFind = 5000;
	VM.ratio = 1.5; // 只搜索vmSourceOrder的一部分
	VM.maxIter = 10; // 迁移最大迭代次数 -- 可微调提高
	VM.fitThreshold = 10; //10; // 微调
	VM.stopTimes = 15;
	VM.delayTimes = 35; // 越大越不易解锁
	VM.delCntMax = 1500;
	request.delLarge = 300;

	/*读取非请求部分*/
	infoDataIn(cin, server, VM, request);

	/*初始化，排序等操作*/
	request.initReqNum(); // 需要统计每天的请求数
	server.genInfoV();// 生成server.infoV，向量，为了写多线程
	server.getMeanPerCost();

	for (int iDay = 0; iDay < request.dayNum; iDay++) {
#ifdef LOCAL
		cout << iDay << endl;
		TIMEend = clock();
		cout<<(double)(TIMEend-TIMEstart)/CLOCKS_PER_SEC << 's' << endl;
#endif
		/*读取请求部分*/
		if (!request.readOK)
			reqDataIn(cin, request, iDay);

#ifdef GAME
		ofstream fout;
		fout.open("game.txt", ios::app);
		reqDataOut(request, fout, iDay);
#endif // GAME

		/*获取虚拟机的额外信息*/
		VM.getExtraInfo(iDay, request);

		/******* 决赛新增内容 ******/
		quoteStrategy(iDay, request, VM, server);
		dataOutQuote(iDay, VM, request);  // 输出我方定价
		VM.updateRequest(iDay, request, cin);  // 这里删除拿不到的请求

		/*请求排序*/
		vector<sRequestItem> priRequest = request.info[iDay];
		orderRequest(VM, request, iDay);

		/*购买 迁移 部署 删除*/
		sspEachDay(iDay, server, VM, request);

		/*分析对手出价*/
		VM.analyseQuote(request, iDay);

		/*服务器的额外信息*/
		server.serExtraServerInfo(iDay);

		/*id 映射*/
		server.idMappingEachDay(iDay);

		/*恢复请求原始排序*/
		request.info[iDay] = priRequest;

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

	/*输出迁移次数*/
	int transCnt = 0;
	for (auto &x : VM.transVmRecord) {
		transCnt += (int)x.size();
	}
	cout << "迁移次数：" << transCnt << endl;

	fin.close();
#endif
	
	return 0;
}
