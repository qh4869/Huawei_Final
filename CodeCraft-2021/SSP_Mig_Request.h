#pragma once
#include "Request.h"

class cSSP_Mig_Request : public cRequests {
public:
	/*统计每天的add和del请求个数，不能调用cVM中的方法，因为只能读取部分天数的数据*/
	vector<int> addNum;
	vector<int> delNum;
	void getReqNumEachDay(int iDay);
	void initReqNum();

	int delLarge = 200; // 大于这个数表示del请求很多
};