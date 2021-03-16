#include "tools.h"

// ���뵥�������ʱ���ҵ��ܷŽ��������������˵ķ�����
sServerItem chooseServer(cServer &server, sVmItem &requestVM) {

	int minCost = INT_MAX;
	sServerItem myServer;

	// myServerSet���з�����ʱ�������ѹ���ķ�����
	if (server.myServerSet.size() > 0) {

		sMyEachServer tempServer;
		int serID;
		for (unsigned i = 0; i < server.myServerSet.size(); i++) {

			serID = server.resourceOrder[i].first;
			tempServer = server.myServerSet[serID];
			/// ������������ǲ������////////////////////////////////////
			//serID = i;   // server ID ���Ǵ洢��˳��
			//tempServer = server.myServerSet[i];    // �����еķ������а�˳��鿴�Ƿ��������
			/////////////////////////////////////
			
			if (!requestVM.nodeStatus) {  // ���ڵ�

				if (tempServer.aIdleCPU >= requestVM.needCPU && tempServer.aIdleRAM >= requestVM.needRAM) {   // a �ڵ�
					myServer.energyCost = -1;
					myServer.hardCost = -1;
					myServer.buyID = serID;
					myServer.node = true;
					return myServer;
				}
				else if (tempServer.bIdleCPU >= requestVM.needCPU && tempServer.bIdleRAM >= requestVM.needRAM) { // b �ڵ�
					myServer.energyCost = -1;
					myServer.hardCost = -1;
					myServer.buyID = serID;
					myServer.node = false;
					return myServer;
				}

			}
			else {   // ˫�ڵ�

				if (tempServer.aIdleCPU >= requestVM.needCPU / 2 && tempServer.aIdleRAM >= requestVM.needRAM / 2
					&& tempServer.bIdleCPU >= requestVM.needCPU / 2 && tempServer.bIdleRAM >= requestVM.needRAM / 2) {
					myServer.energyCost = -1;
					myServer.hardCost = -1;
					myServer.buyID = serID;
					return myServer;
				}

			}

		}

	}

	// �ѹ����������û�����������ģ���Ҫ���µķ�����
	for (int i = 0; i < server.priceOrder.size(); i++) {

		myServer = server.info[server.priceOrder[i].first];   // ��������ȡ��������
		if (!requestVM.nodeStatus && requestVM.needCPU <= myServer.totalCPU / 2   // ���ڵ�
			&& requestVM.needRAM <= myServer.totalRAM / 2) {
			return myServer;
		}
		else if (requestVM.nodeStatus && requestVM.needCPU <= myServer.totalCPU  // ˫�ڵ�
			&& requestVM.needRAM <= myServer.totalRAM) {
			return myServer;
		}

	}

	return myServer;

}


// �ж����������鿴�Ƿ��з��������ķ��������еĻ������Ҽ۸�����˵��Ǹ�
sServerItem chooseServer(cServer &server, unordered_map<string, sVmItem> &workVm, vector<pair<string, int>> &restID, int begin) {

	vector<sVmItem> vmSet;     // ���ڼ�¼��Ҫ����������
	string id;     // �����ID
	for (unsigned i = begin; i < restID.size() - 1; i++) {
		id = restID[i].first;    // restID: id->index
		vmSet.push_back(workVm[id]);  // workVM: id->sVmItem
	}

	int minCost = INT_MAX;
	sServerItem myServer;

	// ������˵Ŀ�ʼ��
	for (int i = 0; i < server.priceOrder.size(); i++) {
		myServer = server.info[server.priceOrder[i].first];    // ��������ȡ��������
		if (isFitServer(myServer, vmSet)) {    // ����ҵ��˵Ļ�ֱ�ӷ��أ���ʱ�۸��Ѿ�����͵���
			return myServer;
		}
	}

	myServer.serName = "";   // ��ʾ�Ҳ���
	return myServer;

}


// �ж�����������Ƿ��ܼ���ָ���ķ����������Է���true������Ϊfalse
bool isFitServer(sMyEachServer &myServer, vector<sVmItem> &vmSet) {

	// �ȴ���˫�ڵ�
	for (unsigned i = 0; i < vmSet.size(); i++) {
		if (vmSet[i].nodeStatus) {  // true��ʾ˫�ڵ�
			myServer.aIdleCPU -= vmSet[i].needCPU / 2;
			myServer.bIdleCPU -= vmSet[i].needCPU / 2;
			myServer.aIdleRAM -= vmSet[i].needRAM / 2;
			myServer.bIdleRAM -= vmSet[i].needRAM / 2;
			if (myServer.aIdleCPU < 0 || myServer.bIdleCPU < 0
				|| myServer.aIdleRAM < 0 || myServer.bIdleRAM < 0) {
				return false;
			}
		}
	}

	// ���������ڵ�
	for (unsigned i = 0; i < vmSet.size(); i++) {
		if (!vmSet[i].nodeStatus) {   // false��ʾ���ڵ�
			if (myServer.aIdleCPU - vmSet[i].needCPU >= 0 && myServer.aIdleRAM - vmSet[i].needRAM >= 0) {
				myServer.aIdleCPU -= vmSet[i].needCPU;
				myServer.aIdleRAM -= vmSet[i].needRAM;
			}
			else if (myServer.bIdleCPU - vmSet[i].needCPU >= 0 && myServer.bIdleRAM - vmSet[i].needRAM >= 0) {
				myServer.bIdleCPU -= vmSet[i].needCPU;
				myServer.bIdleRAM -= vmSet[i].needRAM;
			}
			else {
				return false;
			}
		}
	}

	return true;

}


bool isFitServer(sServerItem &serverItem, vector<sVmItem> &vmSet) {

	int aCpu = serverItem.totalCPU / 2;
	int bCpu = serverItem.totalCPU / 2;
	int aRam = serverItem.totalRAM / 2;
	int bRam = serverItem.totalRAM / 2;

	// �ȴ���˫�ڵ�
	for (unsigned i = 0; i < vmSet.size(); i++) {
		if (vmSet[i].nodeStatus) { // true��ʾ˫�ڵ�
			aCpu -= vmSet[i].needCPU / 2;
			bCpu -= vmSet[i].needCPU / 2;
			aRam -= vmSet[i].needRAM / 2;
			bRam -= vmSet[i].needRAM / 2;

			if (aCpu < 0 || bCpu < 0 || aRam < 0 || bRam < 0) {
				return false;
			}
		}
	}

	// ���������ڵ�
	for (unsigned i = 0; i < vmSet.size(); i++) {
		if (!vmSet[i].nodeStatus) {   // ���ڵ㴦��
			if (aCpu - vmSet[i].needCPU >= 0 && aRam - vmSet[i].needRAM >= 0) {
				aCpu -= vmSet[i].needCPU;
				aRam -= vmSet[i].needRAM;
			}
			else if (bCpu - vmSet[i].needCPU >= 0 && bRam - vmSet[i].needRAM >= 0) {
				bCpu -= vmSet[i].needCPU;
				bRam -= vmSet[i].needRAM;
			}
			else {
				return false;
			}
		}
	}

	return true;

}

// �Ž����칺���������VM���𣬷Ž��ѹ����������VM������֮ǰ�Ѿ����
void deployVM(cServer &server, cVM &VM, const cRequests &request, int index, int begin, int end, vector<bool> &hasDeploy,
	int whichDay, int baseNum) {

	// �ȴ�˫�ڵ㿪ʼ����Ȼ�����ǵ��ڵ�
	for (int iTerm = begin; iTerm < end; iTerm++) {

		if (hasDeploy[iTerm - baseNum]) {  // ����Ѿ�������
			continue;
		}

		sRequestItem requestTerm = request.info[whichDay][iTerm];
		sVmItem requestVm = VM.info[requestTerm.vmName];
		if (requestVm.nodeStatus) {   // true��ʾ˫�ڵ�
			//VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index);
			VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index, iTerm);
		}

	}

	// ���𵥽ڵ�
	for (int iTerm = begin; iTerm < end; iTerm++) {

		if (hasDeploy[iTerm - baseNum]) {  // ����Ѿ�������
			continue;
		}

		sRequestItem requestTerm = request.info[whichDay][iTerm];
		sVmItem requestVm = VM.info[requestTerm.vmName];
		if (!requestVm.nodeStatus) {   // ��ʾ���ڵ�
			//int flag = VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index, true);
			int flag = VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index, true, iTerm);
			if (flag == 2) {
				//VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index, false);
				VM.deploy(server, whichDay, requestTerm.vmID, requestTerm.vmName, index, false, iTerm);
			}
		}

	}

}


// ÿ�����֮��Ϊ�¹���ķ���������IDӳ���
void deployServerID(cServer &server, int whichDay) {

	int begin = 0;
	int serverNum = server.dayServerNum[whichDay];
	int count = 0;   // ���ڼ�¼�������������ֵ
	for (int i = whichDay - 1; i >= 0; i--) {
		begin += server.dayServerNum[i];   // ��һ��ı�ŵ���Сֵ�϶���ǰ���칺�������������
	}

	unordered_map<string, int> record = server.buyRecord[whichDay];
	string serName;
	for (auto ite = record.begin(); ite != record.end(); ite++) {

		serName = ite->first;
		for (int i = begin; i < begin + serverNum; i++) {
			if (serName == server.myServerSet[i].serName) {    // ����ҵ���ͬ�ķ�����
				server.virToReal.insert({ i, count + begin });  // ����ID->��ʵID
				server.realToVir.insert({ count + begin, i });  // ��ʵID->����ID
				count++;
			}
		}

	}

}


// ����ʱ�������������ģ�������������������¼
void deployRecord(cServer &server, cVM &VM, const cRequests &request, int whichDay) {

	vector<sDeployItem> dayRecord = VM.deployRecord[whichDay];  // ����ļ�¼
	vector<sDeployItem> realRecord(request.numEachDay[whichDay]);   // ʵ�ʵļ�¼
	sDeployItem defaultItem;
	defaultItem.serID = -1;
	for (int i = 0; i < request.numEachDay[whichDay]; i++) {
		realRecord[i] = defaultItem;   // ɾ������ʱserIDΪ-1
	}
	sDeployItem temp;
	int serID;
	int realIndex;
	for (int i = 0; i < dayRecord.size(); i++) {
		temp = dayRecord[i];
		realIndex = temp.indexTerm;
		realRecord[realIndex].node = temp.node;      // node
		realRecord[realIndex].isSingle = temp.isSingle;   // isSingle
		realRecord[realIndex].serID = server.virToReal[temp.serID];    // ��¼��ʵֵ
	}
	VM.realDeployRecord.push_back(realRecord);


}
