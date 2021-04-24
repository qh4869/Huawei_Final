#include "Pricer.h"
#include <fstream>
#include <cstdlib>

bool planB = false;

void cPricer::updateRate(int iDay) {
/* Fn: 调价
* Note:
*	- 每次运行迭代调整controlRatio，并输出对应的genRatio
*/

	/*统计当日的最低折扣*/
	int min;
	auto minIte = min_element(minRatio.begin(), minRatio.end());
	if (minIte == minRatio.end()) // minRatio is empty，说明这天就没有add请求
		return;
	else
		min = *minIte;

	if (min > maxRatio) { // 顾客当日所有报价 都比 我的各个成本价还高
		/*controlRatio 跟上一天的一样*/
		// 其实这里genRatio等于什么无所谓，出价的时候必然一个都不卖，只是为了程序可读性而已
		genRatio = min + controlRatio; 
	}
	else {
		/*根据state调价*/
		controlRatio += fixRate.at(state);
		genRatio = min + controlRatio;
		if (genRatio < min) {
			genRatio = min;
			controlRatio = 0;
		}
		else if (genRatio > maxRatio) {
			genRatio = maxRatio;
			controlRatio = maxRatio - min;
		}
	}

	// /*debug*/
	// ofstream fout;
	// fout.open("tmpOut.txt", ios::app);
	// if (iDay > 0)
	// 	fout << "lastDay:" << iDay - 1 << " " << compDiscount.at(iDay-1) << " -> ";

	// /*预测对手平均折扣*/
	// if (iDay > validDays) { // 有足够多的天数数据，才能开始预测对手
	// 	double predictedQuote = predictCompNextQuote(); 
	// 	fout << "today:" << iDay << " " << predictedQuote << endl;
	// }
	// else
	// 	fout << endl;
	// if (predictedQuote < min) { // 对手报价可能比我的成本价还低，
	// 	genRatio = min; // 那我就出成本价
	// 	controlRatio = 0;
	// }
	// else if (predictedQuote < maxRatio && genRatio > predictedQuote) { // 预测报价在max和min之间，而且比我要出的价还低
	// 	genRatio = predictedQuote; // 跟他一样，因为预测有误差，调整也没啥意义还可能亏钱
	// 	controlRatio = genRatio - min;
	// }

}

void cPricer::updatePseudoVar(cServer &server) {
	pseudoServerSet = server.myServerSet;
}

int cPricer::pseudoFfDouble(int reqCPU, int reqRAM) {
/* Fn: 双节点搜索已有服务器-pseudo
*/
	for (int iSer = 0; iSer < (int)pseudoServerSet.size(); iSer++) {
		if (pseudoServerSet[iSer].aIdleCPU >= reqCPU / 2 && pseudoServerSet[iSer].aIdleRAM >= reqRAM / 2 \
			&& pseudoServerSet[iSer].bIdleCPU >= reqCPU / 2 && pseudoServerSet[iSer].bIdleRAM >= reqRAM / 2)
			return iSer;
	}

	return -1;
}

std::tuple<int, bool> cPricer::pseudoFfSingle(int reqCPU, int reqRAM) {
/* Fn: 单节点搜索已有服务器-pseudo
*/
	for (int iSer = 0; iSer < (int)pseudoServerSet.size(); iSer++) {
		if (pseudoServerSet[iSer].aIdleCPU >= reqCPU && pseudoServerSet[iSer].aIdleRAM >= reqRAM)
			return {iSer, true};
		if (pseudoServerSet[iSer].bIdleCPU >= reqCPU && pseudoServerSet[iSer].bIdleRAM >= reqRAM)
			return {iSer, false};
	}

	return {-1, false};
}

void cPricer::pseudoDeploy(cVM &VM, string vmName, int serID) {
/* Fn: 双节点伪部署
*/
	int needCPU = VM.reqCPU(vmName);
	int needRAM = VM.reqRAM(vmName);

	pseudoServerSet[serID].aIdleCPU -= needCPU / 2;
	pseudoServerSet[serID].aIdleRAM -= needRAM / 2;
	pseudoServerSet[serID].bIdleCPU -= needCPU / 2;
	pseudoServerSet[serID].bIdleRAM -= needRAM / 2;
}

void cPricer::pseudoDeploy(cVM &VM, string vmName, int serID, bool serNode) {
/* Fn: 单节点伪部署
*/
	int needCPU = VM.reqCPU(vmName);
	int needRAM = VM.reqRAM(vmName);

	if (serNode) {
		pseudoServerSet[serID].aIdleCPU -= needCPU;
		pseudoServerSet[serID].aIdleRAM -= needRAM;
	}
	else {
		pseudoServerSet[serID].bIdleCPU -= needCPU;
		pseudoServerSet[serID].bIdleRAM -= needRAM;
	}
}

int cPricer::pseudoPurchase(cServer &server, string serName) {
/* Fn: 伪购买
*/
	sMyEachServer oneServer;

	oneServer.serName = serName;
	oneServer.aIdleCPU = server.info[serName].totalCPU / 2;
	oneServer.aIdleRAM = server.info[serName].totalRAM / 2;
	oneServer.bIdleCPU = server.info[serName].totalCPU / 2;
	oneServer.bIdleRAM = server.info[serName].totalRAM / 2;

	pseudoServerSet.push_back(oneServer);

	return pseudoServerSet.size() - 1;
}

void cPricer::setQuote(cVM &VM, cSSP_Mig_Request &request, cSSP_Mig_Server &server, int iDay) {
/* Fn: 有限状态机方案
*/

	/*debug*/
	// ofstream fout;
	// fout.open("tmpOut.txt", ios::app);

	// 初始化
	updatePseudoVar(server);
	minRatio.clear();
	ratioAndQuote.clear();
	double cpuAveRatio, ramAveRatio;
	tie(cpuAveRatio, ramAveRatio) = server.getAveResourceRatio(); // 计算占空比(之前所有天的均值，不考虑空SV)
	double cpuAveRatioIncAll, ramAveRatioIncAll;
	tie(cpuAveRatioIncAll, ramAveRatioIncAll) = server.getAveResourceRatioIncludeALL();
	if (iDay == veryEarlyDay && veryEarlyExtended == false) {
		moreVeryEarlyDay(request); // veryEarlyDay判断顺延
	}

	for (int iTerm=0; iTerm<request.numEachDay[iDay]; iTerm++) {
		string vmID = request.info[iDay][iTerm].vmID;
		if (request.info[iDay][iTerm].type) { // add
			string vmName = request.info[iDay][iTerm].vmName;
			bool vmIsDouble = VM.isDouble(vmName);
			int vmReqCPU = VM.reqCPU(vmName);
			int vmReqRAM = VM.reqRAM(vmName);
			int lifeTime = request.info[iDay][iTerm].lifetime;
			int userQuote = request.info[iDay][iTerm].quote;
			int serID;
			bool serNode;

			/*所有请求的用户出价求和*/
			int userTotalQuote = userQuote;
			int estEnergyCost;
			int estHardCost;

			if (vmIsDouble) {
				serID = pseudoFfDouble(vmReqCPU, vmReqRAM);
			}
			else {
				tie(serID, serNode) = pseudoFfSingle(vmReqCPU, vmReqRAM);
			}

			if (serID != -1) { // 装入已有服务器
				if (vmIsDouble)
					pseudoDeploy(VM, vmName, serID);
				else
					pseudoDeploy(VM, vmName, serID, serNode);
				/*每天平摊硬件成本*/ 
				string serName = pseudoServerSet[serID].serName;
				double hardTax, hardBuyTax;
				int purDate;
				if (server.purchaseDate.count(serID)) // 不是当日伪购买的
					purDate = server.purchaseDate[serID];
				else
					purDate = iDay;
				if (purDate != iDay) { // 直接就能放入已有的服务器
					hardBuyTax = 1;
					if (purDate <= earlyDay) { // 前期购入
						hardTax = hardTaxEarly;
					}
					else { // 中期或者后期购入
						hardTax = hardTaxMid;
					}
				}
				else { // 当天伪购买，同下面伪购买的策略
					if (iDay <= earlyDay) {
						hardTax = hardTaxEarly;
						hardBuyTax = 1;
					}
					else if (iDay <= laterDay) {
						hardTax = hardTaxMid;
						hardBuyTax = 1;
					}
					else {
						hardTax = hardTaxMid;
						hardBuyTax = hardTaxLaterBuy;
					}
				}

				if (cpuAveRatioIncAll != -1 && ramAveRatioIncAll != -1) {
					estHardCost = ((double)vmReqCPU * server.CPUHardCost / cpuAveRatioIncAll \
						+  (double)vmReqRAM * server.RAMHardCost / ramAveRatioIncAll) \
						/ (request.dayNum - purDate) * lifeTime * hardTax * hardBuyTax;
				}
				else {
					estHardCost = ((double)vmReqCPU * server.CPUHardCost +  (double)vmReqRAM * server.RAMHardCost) \
						/ (request.dayNum - purDate) * lifeTime * hardTax * hardBuyTax;
				}
				/*平摊功耗成本*/
				if (cpuAveRatio != -1 && ramAveRatio != -1) {
					estEnergyCost = ((double)vmReqCPU * server.CPUEnergyCost / cpuAveRatio \
						+ (double)vmReqRAM * server.RAMEnergyCost / ramAveRatio) \
						* lifeTime;
				}
				else {
					estEnergyCost = ((double)vmReqCPU * server.CPUEnergyCost + (double)vmReqRAM * server.RAMEnergyCost) \
						* lifeTime;
				}
			}
			else {
				string serName = server.chooseSer(vmReqCPU, vmReqRAM, vmIsDouble);
				serID = pseudoPurchase(server, serName);
				if (vmIsDouble)
					pseudoDeploy(VM, vmName, serID);
				else
					pseudoDeploy(VM, vmName, serID, true);
				/*每天平摊硬件成本*/
				double hardTax;
				double hardBuyTax;
				if (iDay <= earlyDay) { // 前期
					hardTax = hardTaxEarly;
					hardBuyTax = 1;
				}
				else if (iDay <= laterDay){ // 中期
					hardTax = hardTaxMid;
					hardBuyTax = 1;
				}
				else { // 后期
					hardTax = hardTaxMid;
					hardBuyTax = hardTaxLaterBuy;
				}

				if (cpuAveRatioIncAll != -1 && ramAveRatioIncAll != -1) {
					estHardCost = ((double)vmReqCPU * server.CPUHardCost / cpuAveRatioIncAll \
						+ (double)vmReqRAM * server.RAMHardCost / ramAveRatioIncAll) \
						/ (request.dayNum - iDay) * lifeTime * hardTax * hardBuyTax;
				}
				else {
					estHardCost = ((double)vmReqCPU * server.CPUHardCost + (double)vmReqRAM * server.RAMHardCost) \
						/ (request.dayNum - iDay) * lifeTime * hardTax * hardBuyTax;
				}
				/*平摊功耗成本*/
				if (cpuAveRatio != -1 && ramAveRatio != -1) {
					estEnergyCost = ((double)vmReqCPU * server.CPUEnergyCost / cpuAveRatio \
						+ (double)vmReqRAM * server.RAMEnergyCost / ramAveRatio) \
						* lifeTime;
				}
				else {
					estEnergyCost = ((double)vmReqCPU * server.CPUEnergyCost + (double)vmReqRAM * server.RAMEnergyCost) \
						* lifeTime;
				}			
			}

			if (iDay <= veryEarlyDay) {
				double a = (double)(estHardCost + estEnergyCost) / userTotalQuote * estCostScale * disCountVeryEarly;
				minRatio.push_back(a);
				ratioAndQuote.push_back( {vmID, {a, userTotalQuote}});
			}
			else {
				double a = (double)(estHardCost + estEnergyCost) / userTotalQuote * estCostScale;
				minRatio.push_back(a);
				ratioAndQuote.push_back({vmID, {a, userTotalQuote}});
			}
		}
	}

	///////////////bind
	bind(iDay, VM);


	if (iDay == 0) { // 初期争夺用户时期完全按照最低价出价
		genRatio = -1; // -1是个标记，即按照minRatio定价
	}
	else { // 之后的每一天要调价
		// 前一天的胜率
		double winRatio = VM.winNum.at(iDay-1) / (VM.winNum.at(iDay-1) + VM.lossNum.at(iDay-1));

		/*判断win/loss反转*/
		if (winRatio < lossValue) {
			if (!deter) { // 第1天
				state = 1;
				curstate = true; // 上一天博弈失败比较多
				deter = true;
			}
			else if (curstate == false){
				curstate = true;
				toggle = true;
			}
		}
		else if (winRatio > winValue) {
			if (!deter) {
				state = 0;
				curstate = false; // 上一天博弈成功比较多
				deter = true;
			}
			else if (curstate == true) {
				curstate = false;
				toggle = true;
			}
		}

		/*state2的计数器*/
		if (state == 2)
			cnt++;

		/*状态转移*/
		switch (state) {
			case 0: // 涨价状态
				if (toggle) {
					toggle = false;
					state = 1;	
					curstate = true;
				}
				break;
			case 1: // 降价状态
				if (toggle) {
					toggle = false;
					state = 2;
					curstate = false;
					cnt = 0;
				}
				break;
			case 2: // 维持稳定，缓慢上涨
				if (toggle) {
					toggle = false;
					state = 1;
					curstate = true;
				}
				else if (cnt >= 50) { // 50次计数
					state = 0;
					curstate = false;
				}
				break;
		}

		/*调价*/
		updateRate(iDay);
	}

	if (planB)
		genRatio = -1;

	/*定价*/
	setEqualPrice(VM, request, iDay);
}

void cPricer::setEqualPrice(cVM &VM, cSSP_Mig_Request &request, int iDay) {
/* Fn: 出价
*
* Note:
*	- genRatio == -1 表示按照成本价出价
*/
	int cnt = 0;

	unordered_map<string, int> dayQuote;  // 当天的报价
	for (auto &req : request.info[iDay]) {
		if (req.type) { // true : add
			if (genRatio == -1) {
				if (minRatio.at(cnt) <= maxRatio)
					dayQuote.insert({req.vmID, req.quote * minRatio.at(cnt)}); // 按照每个请求的成本价出价
				else
					dayQuote.insert({req.vmID, -1});
			}
			else {
				if (minRatio.at(cnt) > maxRatio) { // 顾客给出的价格比我的成本价还低 -> 不卖
					dayQuote.insert({req.vmID, -1});
				}
				else { // minRatio.at(cnt) <= maxRatio
					if (genRatio < minRatio.at(cnt)) { // 计划出价 低于成本价 -> 按照成本价出价
						dayQuote.insert({req.vmID, req.quote * minRatio.at(cnt)});
					}
					else if (genRatio > maxRatio) { // 出价不能高于顾客的最高价 -> 按照顾客可接受的最高价出价
						dayQuote.insert({req.vmID, req.quote});
					}
					else { // genRatio >= minRatio.at(cnt) && genRatio <= maxRatio
						if (request.addNum.at(iDay) > 500 && genRatio < minRatio.at(cnt) * 1.05) {
							double roll = (double)rand() / RAND_MAX;
							if (roll < 0.25) { // 50%的概率不出这么低的价
								double newRatio = minRatio.at(cnt) * 1.025;
								if (newRatio > maxRatio)
									dayQuote.insert({req.vmID, req.quote});
								else
									dayQuote.insert({req.vmID, req.quote * newRatio});
							}
							else {
								dayQuote.insert({req.vmID, req.quote * genRatio});
							}
						}
						else
							dayQuote.insert({req.vmID, req.quote * genRatio});
					}
				}
			}
			cnt++;
		}
	}
	VM.quote.push_back(dayQuote);
}

void cPricer::updateDayThreadhold(int dayNum) {
	earlyDay = (int)(early * dayNum);
	laterDay = (int)(later * dayNum);
	veryEarlyDay = (int)(veryEarly * dayNum);
}

void cPricer::updateCompDiscount(cRequests &request, cVM &VM, int iDay) {
/* Fn: 计算每天对手的平均折扣，不考虑-1(不购买)情况
* Note:
*	- 如果对手一台都不买，就当做他的折扣是1
*/
	int quoteAdd = 0;
	int custQuoteAdd = 0;

	for (auto &req : request.info[iDay]) {
		if (req.type) {
			string vmID = req.vmID;
			int qt = VM.compQuote[iDay].at(vmID);
			if (qt != -1) {
				quoteAdd += qt;
				custQuoteAdd += req.quote;
			}
		}
	}

	if (custQuoteAdd != 0)
		compDiscount.push_back( (double)quoteAdd / custQuoteAdd);
	else
		compDiscount.push_back(1);
}

int cPricer::autocorr(vector<double> &x, vector<double> &corr, int n)
{
/* Fn: 计算自相关需要考虑从第0天开始的数据吗？
*/
    for(int i=0;i<n;i++) {
        double sum = 0;
        int count = 0;
        int start = ((int)x.size() - validDays > 0) ? (int)x.size()-validDays : 0;
        for(int j=start; j+i<x.size(); j++) {
            sum += x[j] * x[j+i];
            count++;
        }
        corr[i] = sum/count;
    }
    return n;
}

int cPricer::levDur(vector<double>::iterator r, vector<double>::iterator b, vector<double>::iterator a, int l)
{
    double *p =(double*) malloc(l*sizeof(double)*4);
    double *cA = p, *pA = p+l, *cZ = p+2*l, *pZ = p+3*l;

    cA[0] = b[0]/r[0]; cZ[0] = r[1]/r[0];
    for(int k = 2; k<=l ; k++) {
        double *t;
        t = cA; cA = pA; pA = t;
        t = cZ; cZ = pZ; pZ = t;

        double sum = 0;
        for(int i=0;i< k-1; i++) { 
        	sum += r[i+1] * pZ[k-2-i]; 
        }
        double zk1 = r[k] - sum;
        sum = 0;
        for(int i=0;i< k-1; i++) { 
        	sum += r[i+1] * pZ[i]; 
        }
        double zk2 = r[0] - sum;

        double zk = zk1/zk2;
        for(int i=0;i<k-1; i++) { 
        	cZ[i] = pZ[i] - pZ[k-2-i]*zk; 
        }
        cZ[k-1] = zk;

        sum = 0;
        for(int i=0;i< k-1; i++) { 
        	sum += r[i+1] * pA[k-2-i]; 
        }
        double ak = b[k-1] - sum;
        ak /= zk2;

        for(int i=0;i<k-1; i++) { 
        	cA[i] = pA[i] - pZ[k-2-i]*ak; 
        }
        cA[k-1] = ak;
    }
    for(int i=0; i< l; i++) {
        a[i] = cA[i];
    }
    free(p);
    return l;
}

double cPricer::predictCompNextQuote() {
/* Fn: 预测对手下一天的平均折扣 by AR model，目前结果不是很靠谱所以没用上
* 还需要平稳性检验
*/
	vector<double> corr(arOrder+1);
	vector<double> a(arOrder); // ar 系数

	autocorr(compDiscount, corr, arOrder+1); // 计算自相关

	levDur(corr.begin(), corr.begin()+1, a.begin(), arOrder);

	double predictedQuote = 0;
	int vsize = compDiscount.size();
	for (int j=0; j<arOrder; j++) {
		predictedQuote += a.at(j) * compDiscount.at(vsize - 1 - j);
	}

	return predictedQuote;
}

void cPricer::moreVeryEarlyDay(cSSP_Mig_Request &request) {
/* Fn: 初期的天数顺延
* Note:
*	- 如果能够读到请求的天数里有一天add请求超过2500，那就延长到这一天 or 最多6%天
*	- 如果目标顺延日期已经过了，那就不调整veraEarlyDay，但是exteded = true
*/
	for (int day = veryEarlyDay + 1; day < request.toDay && day < (int)(request.dayNum * later); day++) {

		if (request.addNum.at(day) > 2500) {
			disCountVeryEarly += disCountStep;
			if (disCountVeryEarly > disCountMax)
				disCountVeryEarly = disCountMax;

			if (day > veryEarlyDay) {
				veryEarlyDay = day;
			}
			if (veryEarlyDay > (int)(request.dayNum * later)) // 多次延长，直到这个天数
				veryEarlyExtended = true;
			return; 
		}
	}

}

bool comRatio(pair<string, pair<double, int>> i, pair<string, pair<double, int>> j) {
	return i.second.first < j.second.first;
}

void cPricer::bind(int iDay, cVM &VM) {
	sort (ratioAndQuote.begin(), ratioAndQuote.end(), comRatio);

	for (auto &x : ratioAndQuote) {
		if (x.second.second > 10000 && VM.Binders.at(iDay).size() < 3) {
			VM.Binders.at(iDay).insert(x.first);
		}
	}

	if (VM.Binders.at(iDay).size() < 3) { 
		for (auto &x : ratioAndQuote) {
			if (VM.Binders.at(iDay).size() < 3 && !VM.Binders.at(iDay).count(x.first) ) {
				VM.Binders.at(iDay).insert(x.first);
			}
		}
	}

	// ofstream fout;
	// fout.open("tmpOut.txt", ios::app);
	// fout << iDay << "-> " << VM.Binders.at(iDay).size() << endl;
	// for (auto &x : VM.Binders.at(iDay)) {
	// 	fout << x << endl;
	// }

}