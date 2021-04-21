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

	void setQuote(cVM &VM, cSSP_Mig_Request &request, cSSP_Mig_Server &server, int iDay);
	void setEqualPrice(cVM &VM, cRequests &request, int iDay);
	void updateRate(int iDay);

	/*伪部署*/
	vector<sMyEachServer> pseudoServerSet;
	void updatePseudoVar(cServer &server);
	int pseudoFfDouble(int reqCPU, int reqRAM);
	std::tuple<int, bool> pseudoFfSingle(int reqCPU, int reqRAM);
	void pseudoDeploy(cVM &VM, string vmName, int serID);
	void pseudoDeploy(cVM &VM, string vmName, int serID, bool serNode);
	int pseudoPurchase(cServer &server, string serName);

	/*分段成本*/
	int earlyDay;
	int laterDay;
	int veryEarlyDay; //这一天会再次确定一下veryEarlyDay需不需要延长，可能延长至add请求特别多的那一天
	bool veryEarlyExtended = false;
	void updateDayThreadhold(int dayNum);
	void moreVeryEarlyDay(cSSP_Mig_Request &request);

	/*预测对手*/
	vector<double> compDiscount; // 统计对手每天的平均折扣，用于预测
	void updateCompDiscount(cRequests &request, cVM &VM, int iDay);
	int autocorr(vector<double> &x, vector<double> &corr, int n); // 计算自相关
	int levDur(vector<double>::iterator r, vector<double>::iterator b, vector<double>::iterator a, int l);
	double predictCompNextQuote();

	/*自定义参数*/
	/*1、估计成本时（自己和自己博弈）使用的是简单部署算法没有迁移，因此成本估计可能会偏高
	  2、估计了个成本后，如果对手迅速降价，导致自己的服务器占空比迅速降低（占空比分布不平稳），可能收不回成本，考虑这一点成本因子稍微大一丢丢比较好
	  3、Plan B: 调参卡不赔钱的成本价出价，完全不管别的因素
	  4、Plan A: 尽可能准确估价，调价，预测对手，分段成本分摊等各种方法*/
	double estCostScale = 0.9; 
	// 我也不知道为啥：最前期往已购买的服务器里装 成本会高估 - >然后进入前期 已有的服务器越多 估算的成本越低 (这个是应该的)
	double disCountVeryEarly = 0.85;
	// 早期硬件成本不调整
	double hardTaxEarly = 1;
	// 中期硬件成本提价 防止因为后期占空比较小导致硬件成本收不回来(占空比估计后期偏高),后期占空比的影响对前期购买的服务器影响较小，所以不提价
	double hardTaxMid = 1.2; 
	// 后期购买服务器的请求涨价，后期的硬件成本不容易收回来，除了中期开始的硬件成本提价，购买服务器的请求还提价
	double hardTaxLaterBuy = 2.5; 
	double veryEarly = 0.05; // 非常早期，
	double early = 0.25; // 前期天数比例
	double later = 0.75; // 后期天数
	int arOrder = 5; // AR预测阶数
	int validDays = 10; // 只用最多前二十天的数据来训练（担心数据不平稳影响结果）

	
};