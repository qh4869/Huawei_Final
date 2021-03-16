#pragma once
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
	// cyt
	string serName;  // 服务器类型
	int buyID;  // 用于记录已购买服务器的ID
	bool node;   // true : a 节点
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
	vector<sMyEachServer> myServerSet; // 当前已经购买的服务器 服务器id就是vector的index
	vector<sMyEachServer> preSvSet; // 预购买服务器vector，不加入到myServerSet中，cVM::buy函数中清零
	vector<sMyEachServer> mySerCopy; // myServerSet的副本，用于预购买和预部署期间，cVM::postDpWithDel函数中同步
	unordered_map<int, int> idMap; // 新旧id的映射关系

	vector<int> newServerNum; // 用于输出: 每天购买的服务器种类数 
							  // vector<unordered_map<string, int>> buyRecord; // 用于输出: 每天 购买的每种服务器 购买记录 (hash: 服务器型号->数目)
	//vector<vector<pair<string, int>>> buyRecord; // 每天 购买服务器记录
	vector<unordered_map<string, int>> buyRecord; // 每天 购买的每种服务器 购买记录 用于输出 (hash: 服务器型号->数目)

												 // 按照价格排序
	static bool mycomp(pair<string, int> i, pair<string, int> j);
	void rankServerByPrice();
	void rankServerByPrice(bool flag);   // alpha表示权重，flag表示是否是第一次生成，不是的话可以降低一点点复杂度

	// 计算某台服务器是否为开机状态
	bool isOpen(int serID);

	int prePurchase(string name); // 预购买服务器
	//int purchase(string serName, int iDay); // 购买服务器 id按照购买顺序分配（可能不符合输出要求
	void purchase(string serName, int iDay); // 购买服务器
	void buy(int iDay);// 预购买转为正式购买
	
	// First Fit，从0号服务器开始找一台能装进去的服务器ID, 找不到返回-1。
	pair<bool, int> firstFitDouble(int reqCPU, int reqRAM); // 双节点部署
	std::tuple<pair<bool, int>, bool> firstFitSingle(int reqCPU, int reqRAM); // 单节点部署
	// 选购服务器，从某个排序后的服务器列表里，按顺序找能够符合虚拟机要求 而且最便宜的服务器 (不可能找不到)
	string chooseSer(int reqCPU, int reqRAM, bool isDoubleNode);

	// CYT 
	int price;
	// 服务器资源从小到大排序，第一个int表示已购买服务器的id，方便与myServerSet映射，第二表示资源数
	vector<pair<int, int>> resourceOrder;     
	vector<int> dayServerNum;   // 每天增加的服务器数量
	unordered_map<int, int> realToVir; // 真实的ID对应虚拟的ID
	unordered_map<int, int> virToReal; //虚拟的ID对应真实的ID

	static bool rescomp(pair<int, int> i, pair<int, int> j);    // 用于资源的排序
	bool rescomp_equal(pair<int, int> i, pair<int, int> j);
	void rankServerByResource(pair<int, int> newOne);    // 对资源进行排序
	void updateRank(int pos, bool flag);   // 传入更新的位置
};
