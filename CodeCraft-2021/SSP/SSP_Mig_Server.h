#pragma once
#include "SSP_Server.h"

namespace cyt {
	bool mycompID(pair<int, int> i, pair<int, int> j);
}

class cSSP_Mig_Server : public cSSP_Server {
public:
	// 记录服务器中虚拟机数量的排序 : serID->虚拟机占用资源cpu+ram {部署/删除/迁移更新，购买不影响}
	vector<pair<int, int>> vmSourceOrder;
	// 更新vmSourceOrder，部署和删除写在deploy,delete函数中，迁移需要单独调用(cyt写的)
	void updatVmSourceOrder(int needCPU, int needRAM, int serID, bool flag); // flag: true(add), false(delete)
	// // 按照服务器中空闲资源大小的排序，结构同上
	// vector<pair<int, int>> vmTargetOrder; 
	// void updatVmTargetOrder(int needCPU, int needRAM, int serID, bool flag); // flag: true(add), false(delete)
	map<int, map<int, map<int, map<int, vector<int>>>>> vmTarOrder; //
	void updatVmTarOrder(int needCPUa, int needRAMa, int needCPUb, int needRAMb, int serID, bool flag);

	// 每台服务器id对应 虚拟机ID集合(string:VMid, int:0(a),1(b),2(double)) {购买/部署/删除/迁移更新}
	vector<unordered_map<string, int>> serverVMSet;

	/*purchase重载，记录每一台服务器内部的虚拟机*/
	int purchase(string serName, int iDay);

	/*自定义参数*/
	double args[4]; // 迁移 best fit
};