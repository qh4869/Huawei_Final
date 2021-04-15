#include "Pricer.h"
#include <fstream>

void cPricer::updateRate(int winRatio) {
	if (winRatio < lossValue) {
		genRatio -= downRate.at(state);
		genRatio = (genRatio < minRatio)? minRatio : genRatio;
	}
	else if (winRatio > winValue) {
		genRatio += upRate.at(state);
		genRatio = (genRatio > maxRatio)? maxRatio : genRatio;
	}
}

void cPricer::setQuote(cVM &VM, cRequests &request, int iDay) {
/* Fn: 有限状态机方案
*/

	// ofstream fout;
	// fout.open("tmpOut.txt", ios::app);

	if (iDay != 0) {
		// 前一天的胜率
		double winRatio = VM.winNum.at(iDay-1) / (VM.winNum.at(iDay-1) + VM.lossNum.at(iDay-1));

		/*查看win/loss反转，如果winRatio在lossValue和winValue之间，那就不可能win/loss反转*/
		if (winRatio < lossValue) {
			if (!deter) {
				curstate = true;
				deter = true;
			}
			else if (curstate == false){
				curstate = true;
				toggle = true;
			}
		}
		else if (winRatio > winValue) {
			if (!deter) {
				curstate = false;
				deter = true;
			}
			else if (curstate == true) {
				curstate = false;
				toggle = true;
			}
		}

		/*state1/2的计数器*/
		if (state == 1 || state == 2)
			cnt++;

		/*状态转移*/
		switch (state) {
			case 0:
				if (toggle) {
					toggle = false;
					state = 1;
					cnt = 0;		
				}
				break;
			case 1:
				if (toggle) {
					toggle = false;
					state = 2;
					cnt = 0;
				}
				else if (cnt >= 3) { // 三次计数
					state = 0;
				}
				break;
			case 2:
				if (toggle) {
					toggle = false;
					state = 1;
					cnt = 0;
				}
				else if (cnt >= 50) { // 50次计数
					state = 0;
				}
				break;
		}

		/*调价*/
		updateRate(winRatio);
	}

	/*定价*/
	setEqualPrice(VM, request, iDay);
}

void cPricer::setEqualPrice(cVM &VM, cRequests &request, int iDay) {
/* Fn: 同一天按照同比例出价
*/
	unordered_map<string, int> dayQuote;  // 当天的报价
	for (auto &req : request.info[iDay]) {
		if (req.type) { // true : add
			dayQuote.insert({ req.vmID, req.quote * genRatio}); 
		} 
	}
	VM.quote.push_back(dayQuote);
}