#include "Request.h"

void cRequests::getNumEachDay() {
	/* Fn: 计算每天的add/delete请求数量, 结果装入numAddEachDay和numDelEachDay
	*/
	// 与天数有关的vector初始化
	numAddEachDay.resize(dayNum);
	numDelEachDay.resize(dayNum);

	for (int iDay = 0; iDay < dayNum; iDay++) {
		//初始化每天的add和delete请求数量
		numAddEachDay[iDay] = 0;
		numDelEachDay[iDay] = 0;

		for (int iTerm = 0; iTerm < numEachDay[iDay]; iTerm++) {
			//求和
			if (info[iDay][iTerm].type) { // add 请求
				numAddEachDay[iDay]++;
			}
			else { // delete 请求
				numDelEachDay[iDay]++;
			}
		}
	}
}

void cRequests::getVarAdd() {
	/* Fn: 求出Add请求的方差，结果装入 varAdd
	*/
	varAdd = 0;
	double meanAdd = 0;
	for (int iDay = 0; iDay < dayNum; iDay++) {
		meanAdd += numAddEachDay[iDay];
	}
	meanAdd /= dayNum;

	for (int iDay = 0; iDay < dayNum; iDay++) {
		varAdd += (numAddEachDay[iDay] - meanAdd) * (numAddEachDay[iDay] - meanAdd);
		varAdd /= (dayNum - 1);
	}
}

void cRequests::getVarDel() {
	/* Fn: 输出delete请求的方差，结果装入 varDel
	*/
	varDel = 0;
	double meanDel = 0;
	for (int iDay = 0; iDay < dayNum; iDay++) {
		meanDel += numDelEachDay[iDay];
	}
	meanDel /= dayNum;

	for (int iDay = 0; iDay < dayNum; iDay++) {
		varDel += (numDelEachDay[iDay] - meanDel) * (numDelEachDay[iDay] - meanDel);
		varDel /= (dayNum - 1);
	}
}