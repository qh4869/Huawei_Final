#include "graphFit.h"


// 根据图的最短路径来部署
void graphFit(cServer &server, cVM &VM, cRequests &request) {

	// 自适应参数
	double addVar, delVar;
	vector<double> args(4);   // (节点数, ratio)
	tie(addVar, delVar) = request.getVarRequest();
	if (delVar < 7) {
		args[0] = 8;
		args[1] = 3.0;
		args[2] = 0.3;
		args[3] = 1.0;
  	}
	else {
		args[0] = 3;
		args[1] = 1.5;
		args[2] = 0.3;
		args[3] = 1.0;
	}

	int engCostStas = 0; // 计算功耗成本
	int hardCostStas = 0;
	int vmTotalNum;    // 迁移前的虚拟机总量
	string costName;
	unordered_map<string, int> dayWorkingVM;
	unordered_map<string, sEachWorkingVM> dayDeleteVM;   // 当天删除的虚拟机(该虚拟机在已购买的服务器中)
	unordered_map<int, sMyEachServer> delSerSet;   // 用于恢复有删除操作服务器的实际容量

	for (int whichDay = 0; whichDay < request.dayNum; whichDay++) {

		server.serverNum.push_back(0);   // 初始化第N天的服务器数量
		cout << whichDay << endl;
		if (whichDay == 0) {
			server.alpha = request.dayNum;
			server.rankServerByPrice(true);  // 第一天为true
		}
		else {
			server.alpha = request.dayNum - whichDay;
			server.rankServerByPrice(false);
		}

		vmTotalNum = VM.workingVmSet.size();
		dayGraphFit(server, VM, request, whichDay, dayWorkingVM, dayDeleteVM, args);


		delSerSet = recoverDelSerSet(server, VM, dayDeleteVM);
		if (whichDay > 0) {
			migrateVM(server, VM, whichDay, dayWorkingVM, vmTotalNum, delSerSet, args);
		}
		dayWorkingVM.clear();
		dayDeleteVM.clear();
		delSerSet.clear();

		for (int iServer = 0; iServer < (int)server.myServerSet.size(); iServer++) {
			if (server.isOpen(iServer)) {
				costName = server.myServerSet[iServer].serName;
				engCostStas += server.info[costName].energyCost;
			}
		}

	}

	for (int iServer = 0; iServer<(int)server.myServerSet.size(); iServer++) {
		costName = server.myServerSet[iServer].serName;
		hardCostStas += server.info[costName].hardCost;
	}
	cout << "成本: " << engCostStas + hardCostStas << endl;

}


// 进行每天的服务器购买和VM部署
void dayGraphFit(cServer &server, cVM &VM, cRequests &request, int whichDay, unordered_map<string, int> &dayWorkingVM, 
	unordered_map<string, sEachWorkingVM> &dayDeleteVM, vector<double> &args) {

	int totalRequest = request.numEachDay[whichDay];   // 当天的请求总数
	int dividend = (int)args[0];   // 将请求分组，每个分组有dividend条请求 // 10 训练集可达到6.08
	int quotient, remainder;   // 商和余数

	quotient = totalRequest / dividend;   // 整除
	remainder = totalRequest % dividend;  // 求余数

	if (quotient == 0) {   // 商为0，直接分为一组
		subGraphFit(server, VM, request, 0, totalRequest, whichDay, dayWorkingVM, dayDeleteVM, args);
	}
	else {
		int begin = 0, end = dividend;   // 用于指定requestItem的起始和结尾
		while (quotient > 0) {   // 根据商分组，每组requestItem数量为dividend
			subGraphFit(server, VM, request, begin, end, whichDay, dayWorkingVM, dayDeleteVM, args);
			begin = end;
			end = end + dividend;
			quotient--;
		}
		if (remainder != 0) {   // 剩下的Item作为一组
			end = end - dividend + remainder;
			subGraphFit(server, VM, request, begin, end, whichDay, dayWorkingVM, dayDeleteVM, args);
		}
	}

}


// 每天的所有requestItem分为若干组组成子图开始Fit
void subGraphFit(cServer &server, cVM &VM, cRequests &request, int begin, int end,
	int whichDay, unordered_map<string, int> &dayWorkingVM, unordered_map<string, sEachWorkingVM> &dayDeleteVM,
	vector<double> &args) {

	int alpha = server.alpha;       // 价格加权系数
	int maxLoc = end - begin + 1;   // 图节点的最多个数（所以从0开始需多1，无用节点多1，所以比实际条目多2）

									// 图中每个节点的转移矩阵，并初始化（存储对应的server）
	vector<vector<sServerItem>> transfer(maxLoc, vector<sServerItem>(maxLoc));
	sServerItem maxServer;
	maxServer.hardCost = INT_MAX;     // 初始化值为一台价格为无穷的服务器
	for (int i = 0; i < maxLoc; i++) {
		for (int j = 0; j < maxLoc; j++) {
			transfer[i][j] = maxServer;
		}
	}

	// 记录每个节点的最小价格，并初始化
	vector<int> minCost(maxLoc);
	for (int i = 0; i < maxLoc; i++) {
		minCost[i] = 0;
	}

	unordered_map<string, sVmItem> workVm;   // 记录当天还在工作的虚拟机集合（ID->VM）
	vector<pair<string, int>> restID;        // 记录加入新购买服务器的虚拟机ID以及索引（ID->图中索引）
	sServerItem myServer;    // 记录匹配的服务器
	vector<int> path(maxLoc);   // 记录最短路径
	path[0] = 0;   // 初始化
	bool isFirst = true;   // 图中第一个有效节点的最小价格值应该为0
	vector<bool> hasDeploy(maxLoc);   // 用于记录VM是否加入到了已购买的服务器中，是为true

	for (int indexTerm = begin; indexTerm < end + 1; indexTerm++) {   // indexTerm 表示该Term在每天的实际顺序

		int iTerm = indexTerm - begin;    // 表示该Term在图中的实际顺序

										  // 不是最后一个节点才需要处理，最后一个节点无用，暂时不需要处理
		if (iTerm < maxLoc - 1) {
			sRequestItem requestTerm = request.info[whichDay][indexTerm];  // 根据每天实际顺序取出请求
			if (requestTerm.vmID == "622635153") {
				int a = 1;
			}
			if (!requestTerm.type) {   // 表示要删除虚拟机
				hasDeploy[iTerm] = true;    // 删除的虚拟机不需要重新部署(((((bug)))))
				deleteVM(server, VM, requestTerm, workVm, restID, dayDeleteVM);
			}
			else {   // 表示要加入虚拟机
				dayWorkingVM.insert({ requestTerm.vmID, 1 });   // 直接加入
				addVM(server, VM, requestTerm, hasDeploy, transfer, workVm, restID, whichDay, iTerm, indexTerm, args);
			}
		}
		else {    // 处理最后一个节点
			restID.push_back(make_pair("", iTerm));   // 最后一个节点只需要加入其实际位置即可
		}

		// 如果图中存在节点，并且该节点是新加入的才需要进行如下处理
		if (restID.size() > 0 && restID.back().second == iTerm) {
			updateTransfer(server, workVm, restID, transfer, iTerm);     // 更新transfer
			updateMinCost(restID, transfer, path, minCost, iTerm, alpha, isFirst);
		}
		else {
			continue;
		}

	}

	// 全部都确定了就开始买服务器和部署VM
	buyServer(server, VM, request, restID, transfer, hasDeploy, path, maxLoc, whichDay, begin);

}


// 删除VM（可能是当天的服务器，也可能是之前已购买的服务器）
void deleteVM(cServer &server, cVM &VM, sRequestItem &requestTerm, unordered_map<string, sVmItem> &workVm,
	vector<pair<string, int>> &restID, unordered_map<string, sEachWorkingVM> &dayDeleteVM) {

	sEachWorkingVM tempVm = VM.workingVmSet[requestTerm.vmID];  // 删除条目中没有记录虚拟机，所以要通过这种方式取出来
	sVmItem requestVm = VM.info[tempVm.vmName];    // 提取要删除的虚拟机的类型

	auto search = VM.workingVmSet.find(requestTerm.vmID);   // 在已购买服务器中查找
	if (search != VM.workingVmSet.end()) {      // 表示该虚拟机在前几天的服务器里

		int serID = VM.workingVmSet[requestTerm.vmID].serverID;    // 记录该虚拟机放在哪台服务器运行
		dayDeleteVM.insert({ requestTerm.vmID, tempVm });

		if (requestVm.nodeStatus) {   // 双节点
			server.myServerSet[serID].aIdleCPU += requestVm.needCPU / 2;
			server.myServerSet[serID].bIdleCPU += requestVm.needCPU / 2;
			server.myServerSet[serID].aIdleRAM += requestVm.needRAM / 2;
			server.myServerSet[serID].bIdleRAM += requestVm.needRAM / 2;
		}
		else {    // 单节点
			if (VM.workingVmSet[requestTerm.vmID].node) {   // true 表示a节点
				server.myServerSet[serID].aIdleCPU += requestVm.needCPU;
				server.myServerSet[serID].aIdleRAM += requestVm.needRAM;
			}
			else {
				server.myServerSet[serID].bIdleCPU += requestVm.needCPU;
				server.myServerSet[serID].bIdleRAM += requestVm.needRAM;
			}
		}
		server.serverVMSet[serID].erase(requestTerm.vmID);
		VM.workingVmSet.erase(requestTerm.vmID);
		server.updatVmSourceOrder(requestVm, serID, false);   // 删除:更新VM数量排序
	}
	else {    // 删除的虚拟机不在已经购买的服务器中
		workVm.erase(requestTerm.vmID);   // 从工作中的虚拟机集合删除该虚拟机
		int temp = 0;   // 用于查找这个虚拟机在那个位置，找到并删除
		while (restID[temp].first != requestTerm.vmID) {    // first 表示虚拟机ID
			temp++;
		}
		restID.erase(restID.begin() + temp);
	}
}


// 添加虚拟机（可能添加到当天购买的服务器，也可能是已购买的服务器中）
void addVM(cServer &server, cVM &VM, sRequestItem &requestTerm, vector<bool> &hasDeploy, vector<vector<sServerItem>> &transfer,
	unordered_map<string, sVmItem> &workVm, vector<pair<string, int>> &restID, int whichDay, int iTerm, int indexTerm,
	vector<double> &args) {

	sVmItem requestVm = VM.info[requestTerm.vmName];    // 找到需要添加的虚拟机
	sServerItem myServer = chooseServer(server, requestVm, args);   // 找到匹配的服务器

	if (myServer.energyCost < 0) {    // 表示能加入到已购买的服务器

		if (requestVm.nodeStatus) {   // true 表示双节点
			hasDeploy[iTerm] = true;
			VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, myServer.buyID);  // deploy中的record是哈希表
		}
		else {
			hasDeploy[iTerm] = true;
			if (myServer.node) {  // true表示a节点
				VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, myServer.buyID, true);  // a node
			}
			else {  // false : b node
				VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, myServer.buyID, false);  // b node
			}
		}
	}
	else {    // 表示加入到这一轮新购买的服务器
		hasDeploy[iTerm] = false;    // 属于新购买的服务器,还未部署
		workVm.insert({ requestTerm.vmID, requestVm });    // 根据虚拟机ID加入虚拟机
		transfer[iTerm][iTerm + 1] = myServer;    // 更新转移矩阵
		restID.push_back(make_pair(requestTerm.vmID, iTerm));    // 更新restID
	}
}


// 更新transfer的值，即添加图中的边
void updateTransfer(cServer &server, unordered_map<string, sVmItem> &workVm, vector<pair<string, int>> &restID,
	vector<vector<sServerItem>> &transfer, int iTerm) {

	// 图中有多个点时需要进行该操作
	if (restID.size() > 1) {
		sServerItem myServer;
		for (int i = restID.size() - 2; i >= 0; i--) {   // 把一个的逻辑也加了进来
			if (restID[i].second == iTerm - 1) {    // 这个已经在添加服务器的时候更新了
				continue;
			}
			myServer = chooseServer(server, workVm, restID, i);   // 找出一台能够连续放入i到iTerm的服务器
			if (myServer.serName != "") {   // 表示找到了符合要求的服务器
				transfer[restID[i].second][iTerm] = myServer;    // second: index
			}
			else {
				break;    // 这次找不到，后面的就更找不到了，直接退出
			}
		}
	}

}


// 更新最小的价格以及路径
void updateMinCost(vector<pair<string, int>> &restID, vector<vector<sServerItem>> &transfer,
	vector<int> &path, vector<int> &minCost, int iTerm, int alpha, bool &isFirst) {

	int minValue = INT_MAX;
	int index = 0;     // 记录节点的实际索引
	int record = 0;   // 用于记录前向节点
	int temp;    // 用于记录临时价格

	for (int i = 0; i < int(restID.size()); i++) {
		index = restID[i].second;   // second: index
		if (transfer[index][iTerm].hardCost != INT_MAX) {   // 表示存在边
			temp = minCost[index] + transfer[index][iTerm].hardCost + alpha * transfer[index][iTerm].energyCost;
			if (temp < minValue) {
				minValue = temp;
				record = index;
			}
		}
	}
	if (isFirst) {
		minCost[iTerm] = 0;
		path[iTerm] = 0;
		isFirst = false;
	}
	else {
		minCost[iTerm] = minValue;
		path[iTerm] = record;
	}
}


// 购买服务器和部署虚拟机
void buyServer(cServer &server, cVM &VM, cRequests &request, vector<pair<string, int>> &restID,
	vector<vector<sServerItem>> &transfer, vector<bool> &hasDeploy, vector<int> &path, int maxLoc, int whichDay, int begin) {

	vector<int> buy;    // 记录在哪些节点买服务器
	buy.push_back(maxLoc - 1);   // 最后一个节点入栈
	while (true) {
		if (buy.back() == restID[0].second) {   // second: index
			break;
		}
		else {
			buy.push_back(path[buy.back()]);
		}
	}

	vector<sServerItem> buyServer;   // 根据要买的节点，从转移矩阵中提取出记录的服务器
	int first = 0;
	int second = 0;
	for (int i = buy.size() - 1; i >= 1; i--) {
		second = buy[i - 1];
		first = buy[i];
		buyServer.push_back(transfer[first][second]);
	}

	int index = 0;  // 用于记录服务器id的增值
	int baseNum = server.myServerSet.size();   // 新购买的服务器id要在已购买的服务器ID上叠加
	for (int i = buy.size() - 1; i > 0; i--) {

		server.purchase(buyServer[index].serName, whichDay);
		deployVM(server, VM, request, index + baseNum, buy[i] + begin,
			buy[i - 1] + begin, hasDeploy, whichDay, begin);
		int value = server.myServerSet[index + baseNum].aIdleCPU + server.myServerSet[index + baseNum].bIdleCPU
			+ server.myServerSet[index + baseNum].aIdleRAM + server.myServerSet[index + baseNum].bIdleRAM;
		pair<int, int> newOne = make_pair(index + baseNum, value);
		index++;  // 服务器id+1

	}

}