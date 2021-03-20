#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <tuple>
#include <utility>
#include "Request.h"
#include "globalHeader.h"
using namespace std;

struct sServerItem {
	int totalCPU;
	int totalRAM;
	int hardCost; // hardware cost
	int energyCost; // energy Cost per day
	string serName;   // 服务器类型
	int buyID;   // 用于记录已购买服务器的ID
	bool node;   // 判断是a节点部署还是b节点部署，true : a节点
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
	int alpha; // 价格加权参数
	vector<pair<string, int>> priceOrder; // 所有服务器价格从小到大排序，其中储存 <服务器的型号, 加权价格>
	vector<pair<int, int>> myServerOrder; // 已购买服务器的某种排序
	vector<pair<int, double>> myServerOrderDB;
	vector<sMyEachServer> myServerSet; // 当前已经购买的服务器 服务器id就是vector的index

	unordered_map<int, int> idMap; // 新旧id的映射关系

	vector<unordered_map<string, int>> buyRecord; // 用于输出: 每天购买的每种服务器购买记录 服务器型号->数目

												  // 服务器排序
	static bool mycomp(pair<string, int> i, pair<string, int> j);
	static bool mycompID(pair<int, int> i, pair<int, int> j);
	static bool mycompIDDB(pair<int, double> i, pair<int, double> j);
	void rankServerByPrice();
	void rankServerByPrice(bool flag);   // flag表示是否第一次，为true表示第一次，需构造变量，false不是第一次，可减少时间
	void rankMyserverbyRatio();
	void rankMyServerbyOccupied();
	void rankMyServerbyIdle();

	// 计算某台服务器是否为开机状态
	bool isOpen(int serID);

	// First Fit，从0号服务器开始找一台能装进去的服务器ID, 找不到返回-1。
	int firstFitDouble(int reqCPU, int reqRAM); // 双节点部署
	std::tuple<int, bool> firstFitSingle(int reqCPU, int reqRAM); // 单节点部署
	int firstFitByIdleOrdDouble(int reqCPU, int reqRAM);
	std::tuple<int, bool> firstFitByIdleOrdSingle(int reqCPU, int reqRAM);

	// 选购服务器，从某个排序后的服务器列表里，按顺序找能够符合虚拟机要求 而且最便宜的服务器 (不可能找不到)
	string chooseSer(int reqCPU, int reqRAM, bool isDoubleNode);

	// 购买服务器
	int purchase(string serName, int iDay); // 购买服务器 id按照购买顺序分配（可能不符合输出要求

											// id 映射
	int idMapping();

	// CYT
	vector<int> serverNum;   // 每天的服务器数量
	vector<unordered_map<string, int>> serverVMSet;   // 每台服务器对应的虚拟机ID集合(string:VMid, int:0(a),1(b),2(double))
	vector<pair<int, int>> vmSourceOrder;    // 记录服务器中虚拟机数量的排序 : serID->虚拟机资源
	void updatVmSourceOrder(sVmItem &requestVm, int serID, bool flag);   // flag: true(add), false(delete)
};