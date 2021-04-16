#include "io.h"
using namespace std;

void dataIn(string fileName, cServer &server, cVM &VM, cRequests &request) {
/*
* Fn: 读入数据
*
* In:
*   - fileName: txt文件名
*
*/
#ifdef LOCAL
	ifstream fin;
	fin.open(fileName.c_str());
	cin.rdbuf(fin.rdbuf()); // 重定向
#endif

	string inputStr;

	// 服务器种类数
	cin >> inputStr;
	server.serverTypeNum = atoi(inputStr.c_str()); // string to int
	// 服务器每条信息
	for (int iTerm = 0; iTerm<server.serverTypeNum; iTerm++) {
		// server name
		string serName;
		cin >> inputStr;
		inputStr.erase(inputStr.begin()); // 删除左括号
		inputStr.erase(inputStr.end() - 1); // 逗号
		serName = inputStr;

		// cpu
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		server.info[serName].totalCPU = atoi(inputStr.c_str());

		// RAM
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		server.info[serName].totalRAM = atoi(inputStr.c_str());

		// hardware price
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		server.info[serName].hardCost = atoi(inputStr.c_str());

		// energy price
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		server.info[serName].energyCost = atoi(inputStr.c_str());
	}

	// 虚拟机总数
	cin >> inputStr;
	VM.vmTypeNum = atoi(inputStr.c_str());
	// 虚拟机的每条信息
	for (int iTerm = 0; iTerm<VM.vmTypeNum; iTerm++) {
		// vm name
		string vmName;
		cin >> inputStr;
		inputStr.erase(inputStr.begin()); // 删除左括号
		inputStr.erase(inputStr.end() - 1); // 逗号
		vmName = inputStr;

		// vm CPU
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		VM.info[vmName].needCPU = atoi(inputStr.c_str());

		// vm RAM
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		VM.info[vmName].needRAM = atoi(inputStr.c_str());

		// double Node
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		VM.info[vmName].nodeStatus = (inputStr == "1") ? true : false;
	}

	// 请求数据天数
	cin >> inputStr;
	request.dayNum = atoi(inputStr.c_str());
	// 与天数有关的vector初始化
	request.numEachDay.resize(request.dayNum);
	request.info.resize(request.dayNum);
	server.buyRecord.resize(request.dayNum);
	VM.transVmRecord.resize(request.dayNum);
	VM.deployRecord.resize(request.dayNum);
	// 每天的请求
	for (int iDay = 0; iDay<request.dayNum; iDay++) {
		// 每天的请求数目
		cin >> inputStr;
		request.numEachDay[iDay] = atoi(inputStr.c_str());
		request.info[iDay].resize(request.numEachDay[iDay]);

		// 每条的请求信息
		for (int iTerm = 0; iTerm<request.numEachDay[iDay]; iTerm++) {
			// 请求类型
			cin >> inputStr;
			inputStr.erase(inputStr.begin()); // 删除左括号
			inputStr.erase(inputStr.end() - 1); // 逗号
			request.info[iDay][iTerm].type = (inputStr == "add") ? true : false;

			// 请求虚拟机的型号
			if (request.info[iDay][iTerm].type) { // add输入虚拟机型号，del不输入型号
				cin >> inputStr;
				inputStr.erase(inputStr.end() - 1);
				request.info[iDay][iTerm].vmName = inputStr;
			}

			// 请求虚拟机ID
			cin >> inputStr;
			inputStr.erase(inputStr.end() - 1);
			request.info[iDay][iTerm].vmID = inputStr;
		}
	}
}

void dataOut(cServer& server, cVM& VM, const cRequests& request) {
/*
* Fn: 将结果输出到标准输出
*
* In:
*
*/
#ifdef LOCAL
	ofstream fout;
	fout.open("dataOut.txt");
	for (int iDay = 0; iDay < request.dayNum; iDay++) {
		//输出（purchase,Q) , Q表示当天购买的服务器种类数
		fout << "(" << "purchase" << "," << server.buyRecord[iDay].size() << ")" << endl;//也可以用 server.buyRecord[iDay].size()
		//输出(服务器型号，购买数量）
		for (auto ite = server.buyRecord[iDay].begin(); ite != server.buyRecord[iDay].end(); ite++) { //迭代器
			fout << "(" << ite->first << "," << ite->second << ")" << endl;
		}
		//输出（migration,W) , W表示当天要迁移的虚拟机数量
		fout << "(" << "migration," << VM.transVmRecord[iDay].size() << ")" << endl;
		//分别输出W台虚拟机的迁移路径
		for (auto ite = VM.transVmRecord[iDay].begin(); ite != VM.transVmRecord[iDay].end(); ite++) { //迭代器
			if (ite->isSingle) {
				if (ite->node)
					fout << "(" << ite->vmID << "," << server.idMap[ite->serverID] << "," << "A" << ")" << endl;
				else
					fout << "(" << ite->vmID << "," << server.idMap[ite->serverID] << "," << "B" << ")" << endl;
			}
			else
				fout << "(" << ite->vmID << "," << server.idMap[ite->serverID] << ")" << endl;
		}

		//输出当天每条虚拟机部署记录，按照add request的顺序
		for (const auto &reqItem : request.info[iDay]) {
			if (!reqItem.type) // del 请求
				continue;

			string vmID = reqItem.vmID;
			if (!VM.deployRecord[iDay].count(vmID)) {
				fout << "有一条add请求没有被处理" << endl;
				return;
			}

			int serID = server.idMap.at(VM.deployRecord[iDay][vmID].serID);
			bool isSingle = VM.deployRecord[iDay][vmID].isSingle;
			bool node = VM.deployRecord[iDay][vmID].node;

			if (isSingle) {
				if (node)
					fout << "(" << serID << "," << "A" << ")" << endl;
				else
					fout << "(" << serID << "," << "B" << ")" << endl;
			}
			else
				fout << "(" << serID << ")" << endl;
		}
	}
#endif

#ifndef LOCAL
	for (int iDay = 0; iDay < request.dayNum; iDay++) {
		//输出（purchase,Q) , Q表示当天购买的服务器种类数
		cout << "(" << "purchase" << "," << server.buyRecord[iDay].size() << ")" << endl;//也可以用 server.buyRecord[iDay].size()
																						 //输出(服务器型号，购买数量）
		for (auto ite = server.buyRecord[iDay].begin(); ite != server.buyRecord[iDay].end(); ite++) { //迭代器
			cout << "(" << ite->first << "," << ite->second << ")" << endl;
		}
		//输出（migration,W) , W表示当天要迁移的虚拟机数量
		cout << "(" << "migration," << VM.transVmRecord[iDay].size() << ")" << endl;
		//分别输出W台虚拟机的迁移路径
		for (auto ite = VM.transVmRecord[iDay].begin(); ite != VM.transVmRecord[iDay].end(); ite++) { //迭代器
			if (ite->isSingle) {
				if (ite->node)
					cout << "(" << ite->vmID << "," << server.idMap[ite->serverID] << "," << "A" << ")" << endl;
				else
					cout << "(" << ite->vmID << "," << server.idMap[ite->serverID] << "," << "B" << ")" << endl;
			}
			else
				cout << "(" << ite->vmID << "," << server.idMap[ite->serverID] << ")" << endl;
		}

		//输出当天每条虚拟机部署记录，按照add request的顺序
		for (const auto &reqItem : request.info[iDay]) {
			if (!reqItem.type) // del 请求
				continue;

			string vmID = reqItem.vmID;
			if (!VM.deployRecord[iDay].count(vmID)) {
				cout << "有一条add请求没有被处理" << endl;
				return;
			}

			int serID = server.idMap.at(VM.deployRecord[iDay][vmID].serID);
			bool isSingle = VM.deployRecord[iDay][vmID].isSingle;
			bool node = VM.deployRecord[iDay][vmID].node;

			if (isSingle) {
				if (node)
					cout << "(" << serID << "," << "A" << ")" << endl;
				else
					cout << "(" << serID << "," << "B" << ")" << endl;
			}
			else
				cout << "(" << serID << ")" << endl;
		}
	}
#endif
}

void infoDataIn(istream &cin, cServer &server, cVM &VM, cRequests &request) {
/* Fn: 读取服务器和虚拟机信息部分 + 请求天数 + 可读天数 + 相关数据结构resize初始化
*/
	string inputStr;

	// 服务器种类数
	cin >> inputStr;
	server.serverTypeNum = atoi(inputStr.c_str()); // string to int
	// 服务器每条信息
	for (int iTerm = 0; iTerm<server.serverTypeNum; iTerm++) {
		// server name
		string serName;
		cin >> inputStr;
		inputStr.erase(inputStr.begin()); // 删除左括号
		inputStr.erase(inputStr.end() - 1); // 逗号
		serName = inputStr;

		// cpu
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		server.info[serName].totalCPU = atoi(inputStr.c_str());

		// RAM
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		server.info[serName].totalRAM = atoi(inputStr.c_str());

		// hardware price
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		server.info[serName].hardCost = atoi(inputStr.c_str());

		// energy price
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		server.info[serName].energyCost = atoi(inputStr.c_str());
	}

	// 虚拟机总数
	cin >> inputStr;
	VM.vmTypeNum = atoi(inputStr.c_str());
	// 虚拟机的每条信息
	for (int iTerm = 0; iTerm<VM.vmTypeNum; iTerm++) {
		// vm name
		string vmName;
		cin >> inputStr;
		inputStr.erase(inputStr.begin()); // 删除左括号
		inputStr.erase(inputStr.end() - 1); // 逗号
		vmName = inputStr;

		// vm CPU
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		VM.info[vmName].needCPU = atoi(inputStr.c_str());

		// vm RAM
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		VM.info[vmName].needRAM = atoi(inputStr.c_str());

		// double Node
		cin >> inputStr;
		inputStr.erase(inputStr.end() - 1);
		VM.info[vmName].nodeStatus = (inputStr == "1") ? true : false;
	}

	// 总天数
	cin >> inputStr;
	request.dayNum = atoi(inputStr.c_str());

	// 可读天数
	cin >> inputStr;
	request.dayReadable = atoi(inputStr.c_str());

	// 与天数有关的vector初始化
	request.numEachDay.resize(request.dayNum);
	request.info.resize(request.dayNum);
	server.buyRecord.resize(request.dayNum);
	VM.transVmRecord.resize(request.dayNum);
	VM.deployRecord.resize(request.dayNum);
}

void reqDataIn(istream &cin, cRequests &request, int iDay) {
/* Fn: 读取部分天数的请求
*/
	string inputStr;

	/*从第几天开始读*/
	int fromDay = request.toDay;

	/*今天可以读到哪一天(第toDay天不读)*/
	int toDay = iDay + request.dayReadable;
	if (toDay >= request.dayNum) { // 今天可以全部读完
		toDay = request.dayNum;
		request.readOK = true;
	}
	request.toDay = toDay;

	for (int day = fromDay; day < toDay; day++) {
		/*每天的请求数目*/
		cin >> inputStr;
		request.numEachDay[day] = atoi(inputStr.c_str());
		request.info[day].resize(request.numEachDay[day]);

		/*每条的请求信息*/
		for (int iTerm = 0; iTerm < request.numEachDay[day]; iTerm++) {
			/*请求类型*/
			cin >> inputStr;
			inputStr.erase(inputStr.begin()); // 删除左括号
			inputStr.erase(inputStr.end() - 1); // 逗号
			request.info[day][iTerm].type = (inputStr == "add") ? true : false;

			/*请求虚拟机的型号*/
			if (request.info[day][iTerm].type) { // add输入虚拟机型号，del不输入型号
				cin >> inputStr;
				inputStr.erase(inputStr.end() - 1); // 逗号
				request.info[day][iTerm].vmName = inputStr;
			}

			/*请求虚拟机ID*/
			cin >> inputStr;
			inputStr.erase(inputStr.end() - 1); // 逗号
			request.info[day][iTerm].vmID = inputStr;

			/******* 决赛新增内容 *******/
			if (request.info[day][iTerm].type) {  // true : add
				/*请求虚拟机寿命*/
				cin >> inputStr;
				inputStr.erase(inputStr.end() - 1); // 逗号
				request.info[day][iTerm].lifetime = atoi(inputStr.c_str());

				/*请求虚拟机用户报价*/
				cin >> inputStr;
				inputStr.erase(inputStr.end() - 1); // 逗号
				request.info[day][iTerm].quote = atoi(inputStr.c_str());
			}

		}
	}
}

void dataOutEachDay(int iDay, cServer &server, cVM &VM, cRequests &request) {
// #ifndef LOCAL
	//输出（purchase,Q) , Q表示当天购买的服务器种类数
	cout << "(" << "purchase" << "," << server.buyRecord[iDay].size() << ")" << endl;
	//输出(服务器型号，购买数量）
	for (auto ite = server.buyRecord[iDay].begin(); ite != server.buyRecord[iDay].end(); ite++) { //迭代器
		cout << "(" << ite->first << "," << ite->second << ")" << endl;
	}

	//输出（migration,W) , W表示当天要迁移的虚拟机数量
	cout << "(" << "migration," << VM.transVmRecord[iDay].size() << ")" << endl;
	//分别输出W台虚拟机的迁移路径
	for (auto ite = VM.transVmRecord[iDay].begin(); ite != VM.transVmRecord[iDay].end(); ite++) { //迭代器
		if (ite->isSingle) {
			if (ite->node)
				cout << "(" << ite->vmID << "," << server.idMap[ite->serverID] << "," << "A" << ")" << endl;
			else
				cout << "(" << ite->vmID << "," << server.idMap[ite->serverID] << "," << "B" << ")" << endl;
		}
		else
			cout << "(" << ite->vmID << "," << server.idMap[ite->serverID] << ")" << endl;
	}

	//输出当天每条虚拟机部署记录，按照add request的顺序
	for (const auto &reqItem : request.info[iDay]) {
		if (!reqItem.type) // del 请求
			continue;

		string vmID = reqItem.vmID;
		if (!VM.deployRecord[iDay].count(vmID)) {
			cout << "有一条add请求没有被处理" << endl;
			return;
		}

		int serID = server.idMap.at(VM.deployRecord[iDay][vmID].serID);
		bool isSingle = VM.deployRecord[iDay][vmID].isSingle;
		bool node = VM.deployRecord[iDay][vmID].node;

		if (isSingle) {
			if (node)
				cout << "(" << serID << "," << "A" << ")" << endl;
			else
				cout << "(" << serID << "," << "B" << ")" << endl;
		}
		else
			cout << "(" << serID << ")" << endl;
	}
	fflush(stdout);
// #endif

/*#ifdef LOCAL
	ofstream fout;
	fout.open("dataOut.txt", ios::app);

	//输出（purchase,Q) , Q表示当天购买的服务器种类数
	fout << "(" << "purchase" << "," << server.buyRecord[iDay].size() << ")" << endl;
	//输出(服务器型号，购买数量）
	for (auto ite = server.buyRecord[iDay].begin(); ite != server.buyRecord[iDay].end(); ite++) { //迭代器
		fout << "(" << ite->first << "," << ite->second << ")" << endl;
	}

	//输出（migration,W) , W表示当天要迁移的虚拟机数量
	fout << "(" << "migration," << VM.transVmRecord[iDay].size() << ")" << endl;
	//分别输出W台虚拟机的迁移路径
	for (auto ite = VM.transVmRecord[iDay].begin(); ite != VM.transVmRecord[iDay].end(); ite++) { //迭代器
		if (ite->isSingle) {
			if (ite->node)
				fout << "(" << ite->vmID << "," << server.idMap[ite->serverID] << "," << "A" << ")" << endl;
			else
				fout << "(" << ite->vmID << "," << server.idMap[ite->serverID] << "," << "B" << ")" << endl;
		}
		else
			fout << "(" << ite->vmID << "," << server.idMap[ite->serverID] << ")" << endl;
	}

	//输出当天每条虚拟机部署记录，按照add request的顺序
	for (const auto &reqItem : request.info[iDay]) {
		if (!reqItem.type) // del 请求
			continue;

		string vmID = reqItem.vmID;
		if (!VM.deployRecord[iDay].count(vmID)) {
			fout << "有一条add请求没有被处理" << endl;
			return;
		}

		int serID = server.idMap.at(VM.deployRecord[iDay][vmID].serID);
		bool isSingle = VM.deployRecord[iDay][vmID].isSingle;
		bool node = VM.deployRecord[iDay][vmID].node;

		if (isSingle) {
			if (node)
				fout << "(" << serID << "," << "A" << ")" << endl;
			else
				fout << "(" << serID << "," << "B" << ")" << endl;
		}
		else
			fout << "(" << serID << ")" << endl;
	}
	fflush(stdout);
#endif*/
}


/*********************** 决赛新增内容 ***********************/
void dataOutQuote(int iDay, cVM &VM, cRequests &request) {
	for (auto &req : request.info[iDay]) {
		if (req.type)
			cout << VM.quote[iDay][req.vmID] << endl;
	}
	fflush(stdout);
}