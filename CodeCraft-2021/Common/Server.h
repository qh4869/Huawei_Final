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

	unordered_map<int, int> idMap; // 新旧id的映射关系

	vector<unordered_map<string, int>> buyRecord; // 用于输出: 每天购买的每种服务器购买记录 服务器型号->数目								  
	
	// 计算某台服务器是否为开机状态
	bool isOpen(int serID);

	// 购买服务器
	int purchase(string serName, int iDay); // 购买服务器 id按照购买顺序分配（可能不符合输出要求

	// id 映射
	int idMapping();
};