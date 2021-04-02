#include "SSP_Mig_Request.h"

void cSSP_Mig_Request::getReqNumEachDay(int iDay) {
	if (iDay >= toDay) {
		cerr << "不能统计当天的请求数，因为当天的请求还没读取" << endl;
		throw "not readed yet";
	}

	for (auto &x : info[iDay]) {
		if (x.type) // add
			addNum[iDay]++;
		else
			delNum[iDay]++;
	}
}

void cSSP_Mig_Request::initReqNum() {
	addNum.resize(dayNum);
	delNum.resize(dayNum);
}