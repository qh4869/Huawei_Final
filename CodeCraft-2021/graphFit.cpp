#include "graphFit.h"


void graphFit(cServer &server, cVM &VM, const cRequests &request) {

	server.price = 0;
	int engCostStas = 0; // ���㹦�ĳɱ�
	int hardCostStas = 0;
	string costName;
	for (int whichDay = 0; whichDay < request.dayNum; whichDay++) {
		
		if (whichDay == 2) {
			int a = 1;
		}
		//cout << whichDay << endl;
		server.dayServerNum.push_back(0);
		if (whichDay == 0) {
			server.alpha = request.dayNum;
			//server.alpha = 400;
			server.rankServerByPrice(true);
		}
		else {
			server.alpha = request.dayNum - whichDay;
			//server.alpha = 400;
			server.rankServerByPrice(false);
		}

		dayGraphFit(server, VM, request, whichDay);
		deployServerID(server, whichDay);
		deployRecord(server, VM, request, whichDay);

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
	cout << "�ɱ�: " << engCostStas + hardCostStas << endl;

}


// ����ÿ��ķ����������VM����
void dayGraphFit(cServer &server, cVM &VM, const cRequests &request, int whichDay) {

	int totalRequest = request.numEachDay[whichDay];   // �������������
	int dividend = 1;   // ��������飬ÿ��������dividend������
	int quotient, remainder;   // �̺�����
	
	quotient = totalRequest / dividend;   // ����
	remainder = totalRequest % dividend;  // ������

	if (quotient == 0) {   // ��Ϊ0��ֱ�ӷ�Ϊһ��
		subGraphFit(server, VM, request, 0, totalRequest, whichDay);
	}
	else {

		int begin = 0, end = dividend;   // ����ָ��requestItem����ʼ�ͽ�β

		while (quotient > 0) {   // �����̷��飬ÿ��requestItem����Ϊdividend
			subGraphFit(server, VM, request, begin, end, whichDay);
			begin = end;
			end = end + dividend;
			quotient--;
		}
		if (remainder != 0) {   // ʣ�µ�Item��Ϊһ��
			end = end - dividend + remainder;
			subGraphFit(server, VM, request, begin, end, whichDay);
		}
		
	}

}

// ÿ�������requestItem��Ϊ�����������ͼ��ʼFit
void subGraphFit(cServer &server, cVM &VM, const cRequests &request, int begin, int end, int whichDay) {

	int alpha = server.alpha;    // �۸��Ȩϵ��
	int maxLoc = end - begin + 1;   // ͼ�ڵ�������������Դ�0��ʼ���1�����ýڵ��1�����Ա�ʵ����Ŀ��2��

	// ͼ��ÿ���ڵ��ת�ƾ��󣬲���ʼ�����洢��Ӧ��server��
	vector<vector<sServerItem>> transfer(maxLoc, vector<sServerItem>(maxLoc));
	sServerItem maxServer;
	maxServer.hardCost = INT_MAX;     // ��ʼ��ֵΪһ̨�۸�Ϊ����ķ�����
	for (int i = 0; i < maxLoc; i++) {
		for (int j = 0; j < maxLoc; j++) {
			transfer[i][j] = maxServer;
		}
	}

	// ��¼ÿ���ڵ����С�۸񣬲���ʼ��
	vector<int> minCost(maxLoc);
	for (int i = 0; i < maxLoc; i++) {
		minCost[i] = 0; 
	}

	unordered_map<string, sVmItem> workVm;   // ��¼���컹�ڹ�������������ϣ�ID->VM��
	vector<pair<string, int>> restID;        // ��¼�����¹���������������ID�Լ�������ID->ͼ��������
	sServerItem myServer;    // ��¼ƥ��ķ�����
	vector<int> path(maxLoc);   // ��¼���·��
	path[0] = 0;   // ��ʼ��
	bool isFirst = true;   // ͼ�е�һ����Ч�ڵ����С�۸�ֵӦ��Ϊ0
	vector<bool> hasDeploy(maxLoc);   // ���ڼ�¼VM�Ƿ���뵽���ѹ���ķ������У���Ϊtrue

	for (int indexTerm = begin; indexTerm < end + 1; indexTerm++) {   // indexTerm ��ʾ��Term��ÿ���ʵ��˳��

		if (indexTerm == 131) {
			int a = 1;
		}

		int iTerm = indexTerm - begin;    // ��ʾ��Term��ͼ�е�ʵ��˳��

		// �������һ���ڵ����Ҫ�������һ���ڵ����ã���ʱ����Ҫ����
		if (iTerm < maxLoc - 1) {  
			sRequestItem requestTerm = request.info[whichDay][indexTerm];  // ����ÿ��ʵ��˳��ȡ������
			if (!requestTerm.type) {   // ��ʾҪɾ�������
				hasDeploy[iTerm] = true;    // ɾ�������������Ҫ���²���
				deleteVM(server, VM, requestTerm, workVm, restID);
			}
			else {   // ��ʾҪ���������
				addVM(server, VM, requestTerm, hasDeploy, transfer, workVm, restID, whichDay, iTerm, indexTerm);
			}
		}
		else {    // �������һ���ڵ�
			restID.push_back(make_pair("", iTerm));   // ���һ���ڵ�ֻ��Ҫ������ʵ��λ�ü���
		}

		// ���ͼ�д��ڽڵ㣬���Ҹýڵ����¼���Ĳ���Ҫ�������´���
		if (restID.size() > 0 && restID.back().second == iTerm) {
			updateTransfer(server, workVm, restID, transfer, iTerm);     // ����transfer
			updateMinCost(restID, transfer, path, minCost, iTerm, alpha, isFirst);
		}
		else {
			continue;
		}

	}

	server.price += minCost.back();    // ������һֱ���ŵļ۸����alphaֵΪ�ض�ֵʱ

	buyServer(server, VM, request, restID, transfer, hasDeploy, path, maxLoc, whichDay, begin);

}

// ɾ��VM
void deleteVM(cServer &server, cVM &VM, sRequestItem &requestTerm, unordered_map<string, sVmItem> &workVm, vector<pair<string, int>> &restID) {

	sEachWorkingVM tempVm = VM.workingVmSet[requestTerm.vmID];  // ɾ����Ŀ��û�м�¼�����������Ҫͨ�����ַ�ʽȡ����
	sVmItem requestVm = VM.info[tempVm.vmName];    // ��ȡҪɾ���������

	auto search = VM.workingVmSet.find(requestTerm.vmID);
	if (search != VM.workingVmSet.end()) {      // ��ʾ���������ǰ����ķ�������

		int serID = VM.workingVmSet[requestTerm.vmID].serverID;    // ��¼�������������̨����������
		if (requestVm.nodeStatus) {   // ˫�ڵ�
			server.myServerSet[serID].aIdleCPU += requestVm.needCPU / 2;
			server.myServerSet[serID].bIdleCPU += requestVm.needCPU / 2;
			server.myServerSet[serID].aIdleRAM += requestVm.needRAM / 2;
			server.myServerSet[serID].bIdleRAM += requestVm.needRAM / 2;
		}
		else {    // ���ڵ�
			if (VM.workingVmSet[requestTerm.vmID].node) {   // true ��ʾa�ڵ�
				server.myServerSet[serID].aIdleCPU += requestVm.needCPU;
				server.myServerSet[serID].aIdleRAM += requestVm.needRAM;
			}
			else {
				server.myServerSet[serID].bIdleCPU += requestVm.needCPU;
				server.myServerSet[serID].bIdleRAM += requestVm.needRAM;
			}
		}
		//////////////////////////////////////////////////////////////////////////
		server.updateRank(serID, false);   // ��ʾ����������VM
		/////////////////////////////////////////////////////////////////////////
	}
	else {    // ɾ��������������Ѿ�����ķ�������

		workVm.erase(requestTerm.vmID);   // �ӹ����е����������ɾ���������
		int temp = 0;   // ���ڲ��������������Ǹ�λ�ã��ҵ���ɾ��
		while (restID[temp].first != requestTerm.vmID) {    // first ��ʾ�����ID
			temp++;
		}
		restID.erase(restID.begin() + temp);

	}

}

// ����VM
void addVM(cServer &server, cVM &VM, sRequestItem &requestTerm, vector<bool> &hasDeploy, vector<vector<sServerItem>> &transfer,
	unordered_map<string, sVmItem> &workVm, vector<pair<string, int>> &restID, int whichDay, int iTerm, int indexTerm) {

	sVmItem requestVm = VM.info[requestTerm.vmName];    // �ҵ���Ҫ��ӵ������
	sServerItem myServer = chooseServer(server, requestVm);   // �ҵ�ƥ��ķ�����

	if (myServer.energyCost < 0) {    // ��ʾ�ܼ��뵽�ѹ���ķ�����

		if (requestVm.nodeStatus) {   // true ��ʾ˫�ڵ�
			hasDeploy[iTerm] = true;
			//VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, myServer.buyID);
			VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, myServer.buyID, indexTerm);
		}
		else {
			hasDeploy[iTerm] = true;
			//int flag = VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, myServer.buyID, true);  // a �ڵ�
			int flag = VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, myServer.buyID, true, indexTerm);  // a �ڵ�
			if (flag == 2) {
				//VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, myServer.buyID, false);  // b �ڵ�
				VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, myServer.buyID, false, indexTerm);  // b �ڵ�
			}
		}
		///////////////////////////////////////////////////////////////////////////
		server.updateRank(myServer.buyID, true);   // �����
		///////////////////////////////////////////////////////////////////////////
	}
	else {    // ��ʾ���뵽��һ���¹���ķ�����

		hasDeploy[iTerm] = false;    // �����¹���ķ�����
		workVm.insert({ requestTerm.vmID, requestVm });    // ���������ID���������
		transfer[iTerm][iTerm + 1] = myServer;    // ����ת�ƾ���
		restID.push_back(make_pair(requestTerm.vmID, iTerm));    // ����restID

	}

}


// ����transfer��ֵ�������ͼ�еı�
void updateTransfer(cServer &server, unordered_map<string, sVmItem> &workVm, vector<pair<string, int>> &restID,
	vector<vector<sServerItem>> &transfer, int iTerm) {

    // ͼ���ж����ʱ��Ҫ���иò���
	if (restID.size() > 1) {

		sServerItem myServer;
		for (int i = restID.size() - 2; i >= 0; i--) {   // ��һ�����߼�Ҳ���˽���
			if (restID[i].second == iTerm - 1) {    // ����Ѿ�����ӷ�������ʱ�������
				continue;
			}
			myServer = chooseServer(server, workVm, restID, i);
			if (myServer.serName != "") {   // ��ʾ�ҵ��˷���Ҫ��ķ�����
				transfer[restID[i].second][iTerm] = myServer;    // second: index
			}
			else {
				break;    // ����Ҳ���������ľ͸��Ҳ����ˣ�ֱ���˳�
			}
		}

	}

}


// ������С�ļ۸��Լ�·��
void updateMinCost(vector<pair<string, int>> &restID, vector<vector<sServerItem>> &transfer, 
	vector<int> &path, vector<int> &minCost, int iTerm, int alpha, bool &isFirst) {

	int minValue = INT_MAX;
	int index = 0;    // ��¼�ڵ��ʵ������
	int record = 0;   // ���ڼ�¼ǰ��ڵ�
	int temp;    // ���ڼ�¼��ʱ�۸�

	for (int i = 0; i < int(restID.size()); i++) {
		index = restID[i].second;   // second: index
		if (transfer[index][iTerm].hardCost != INT_MAX) {   // ��ʾ���ڱ�
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


// ����������Ͳ��������
void buyServer(cServer &server, cVM &VM, const cRequests &request, vector<pair<string, int>> &restID,
	vector<vector<sServerItem>> &transfer, vector<bool> &hasDeploy, vector<int> &path, int maxLoc, int whichDay, int begin) {

	vector<int> buy;    // ��¼����Щ�ڵ��������
	buy.push_back(maxLoc - 1);   // ���һ���ڵ���ջ
	while (true) {
		if (buy.back() == restID[0].second) {   // second: index
			break;
		}
		else {
			buy.push_back(path[buy.back()]);
		}
	}

	vector<sServerItem> buyServer;   // ��ʾҪ��ķ�����
	int first = 0;
	int second = 0;
	for (int i = buy.size() - 1; i >= 1; i--) {
		second = buy[i - 1];
		first = buy[i];
		buyServer.push_back(transfer[first][second]);
	}

	int index = 0;    
	int baseNum = server.myServerSet.size();   // �¹���ķ�����idҪ���ѹ���ķ�����ID�ϵ���
	for (int i = buy.size() - 1; i > 0; i--) {

		server.purchase(buyServer[index].serName, whichDay);
		deployVM(server, VM, request, index + baseNum, buy[i] + begin,
			buy[i - 1] + begin, hasDeploy, whichDay, begin);
		/////////////////////////////////////////////////////////////////////
		int value = server.myServerSet[index + baseNum].aIdleCPU + server.myServerSet[index + baseNum].bIdleCPU
			+ server.myServerSet[index + baseNum].aIdleRAM + server.myServerSet[index + baseNum].bIdleRAM;
		pair<int, int> newOne = make_pair(index + baseNum, value);
		server.rankServerByResource(newOne);
		/////////////////////////////////////////////////////////////////////////
		index++;

	}

}