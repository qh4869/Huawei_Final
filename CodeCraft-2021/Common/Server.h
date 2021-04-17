#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <tuple>
#include <utility>
#include "Request.h"
#include "globalHeader.h"
#include <map>
#include <numeric>
using namespace std;

struct sServerItem {
	int totalCPU;
	int totalRAM;
	int hardCost; // hardware cost
	int energyCost; // energy Cost per day
};

struct sMyEachServer {
	string serName;
	int aIdleCPU; // 各个node 空闲资源
	int bIdleCPU;
	int aIdleRAM;
	int bIdleRAM;
};

class cServer {
public:
	int serverTypeNum;
	unordered_map<string, sServerItem> info; // 服务器型号 -> 其他信息
	
	vector<sMyEachServer> myServerSet; // 当前已经购买的服务器 服务器id就是vector的index

	vector<unordered_map<string, int>> buyRecord; // 用于输出: 每天购买的每种服务器购买记录 服务器型号->数目								  
	
	// 计算某台服务器是否为开机状态
	bool isOpen(int serID);

	// 购买服务器
	int purchase(string serName, int iDay); // 购买服务器 id按照购买顺序分配（可能不符合输出要求

	/*id 映射*/
	unordered_map<int, int> idMap; // 新旧id的映射关系
	int idMapping();
	/*每日id映射*/
	int startID = 0; // 每天服务器起始编号
	void idMappingEachDay(int iDay);

	// 统计成本
	int engCostStas = 0;
	int hardCostStas = 0;
	void hardCostTotal();
	void energyCostEachDay();
	int getTotalCost();

	double CPUHardCost;    // 单个CPU的硬件成本
	double RAMHardCost;    // 单个RAM的硬件成本
	double CPUEnergyCost;  // 单个CPU的能耗成本
	double RAMEnergyCost;  // 单个RAM的能耗成本
	void getMeanPerCost();  // 获取平均的消耗

	vector<double> cpuRatio; // 每天的cpu占比 <每天部署迁移之后更新>，不考虑空的服务器
	vector<double> ramRatio;
	vector<double> cpuRatioIncAll; // 考虑空的服务器
	vector<double> ramRatioIncAll;
	void updateResourceRatio();
	tuple<double, double> getAveResourceRatio();
	void updateResourceRatioIncludeALL();
	tuple<double, double> getAveResourceRatioIncludeALL();
};