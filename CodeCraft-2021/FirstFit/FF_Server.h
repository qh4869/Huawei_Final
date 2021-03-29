#pragma once
#include "Server.h"

class cFF_Server : public cServer {
public:
	int alpha; // 价格加权参数
	vector<pair<string, int>> priceOrder; // 所有服务器价格从小到大排序，其中储存 <服务器的型号, 加权价格>
	vector<pair<int, int>> myServerOrder; // 已购买服务器的某种排序
	vector<pair<int, double>> myServerOrderDB;

	/*服务器排序*/
	static bool mycomp(pair<string, int> i, pair<string, int> j);
	static bool mycompID(pair<int, int> i, pair<int, int> j);
	static bool mycompIDDB(pair<int, double> i, pair<int, double> j);
	void rankServerByPrice(); // 所有类型服务器，用于购买
	/*已经购买的服务器排序*/
	void rankMyserverbyRatio(); 
	void rankMyServerbyOccupied();
	void rankMyServerbyIdle();

	// First Fit，从0号服务器开始找一台能装进去的服务器ID, 找不到返回-1。
	int firstFitDouble(int reqCPU, int reqRAM); // 双节点部署
	std::tuple<int, bool> firstFitSingle(int reqCPU, int reqRAM); // 单节点部署
	int firstFitByIdleOrdDouble(int reqCPU, int reqRAM);
	std::tuple<int, bool> firstFitByIdleOrdSingle(int reqCPU, int reqRAM);

	// 选购服务器，从某个排序后的服务器列表里，按顺序找能够符合虚拟机要求 而且最便宜的服务器 (不可能找不到)
	string chooseSer(int reqCPU, int reqRAM, bool isDoubleNode);

};