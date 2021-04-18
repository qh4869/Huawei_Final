#pragma once
#include "SSP_Mig_VM.h"
#include "SSP_Mig_Request.h"
#include "SSP_Mig_Server.h"
#include <numeric>
#include <algorithm>

class cPricer {
public:
	// 一般性折扣 <第一天估计成本后要初始化，之后每次调价更新>，-1表示按照每个请求的最低价出价，目前只有第0天或者debug能用到
	double genRatio;
	vector<double> fixRate;
	double controlRatio = 0; // genRatio = min + controlRatio，调价控制contrilRatio
	int state = 0;
	bool deter = false; // 标记第1天，第0天进不去调价循环
	bool curstate;
	bool toggle = false;
	double winValue = 0.7;
	double lossValue = 0.3;
	// double minRatio = 0.5;
	vector<double> minRatio; // 对应于每个add请求的成本折扣 <定价估计成本时初始化>
	double maxRatio = 1;
	int cnt;

	cPricer() : genRatio(-1), fixRate({0.01, -0.01, 0.0005}) {
	}

	void setQuote(cVM &VM, cRequests &request, cSSP_Mig_Server &server, int iDay);
	void setEqualPrice(cVM &VM, cRequests &request, int iDay);
	void updateRate();

	/*伪部署*/
	vector<sMyEachServer> pseudoServerSet;
	void updatePseudoVar(cServer &server);
	int pseudoFfDouble(int reqCPU, int reqRAM);
	std::tuple<int, bool> pseudoFfSingle(int reqCPU, int reqRAM);
	void pseudoDeploy(cVM &VM, string vmName, int serID);
	void pseudoDeploy(cVM &VM, string vmName, int serID, bool serNode);
	int pseudoPurchase(cServer &server, string serName);

	/*自定义参数*/
	/*1、估计成本时（自己和自己博弈）使用的是简单部署算法没有迁移，因此成本估计可能会偏高
	  2、估计了个成本后，如果对手迅速降价，导致自己的服务器占空比迅速降低（占空比分布不平稳），可能收不回成本，考虑这一点成本因子稍微大一丢丢比较好
	  3、Plan B: 调参卡不赔钱的成本价出价，完全不管别的因素
	  4、Plan A: 尽可能准确估价，调价，预测对手，分段成本分摊等各种方法*/
	double estCostScale = 0.965; 
	double hardTax0 = 1; // 需要购买服务器的请求 更多提价，因为这个SV空闲天数 是没有均摊价格的VM
	double hardTax1 = 1; // 后20%天数，购买服务器的请求涨价
	
};