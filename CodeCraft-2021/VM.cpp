#include "VM.h"


int cVM::deploy(cServer &server, int iDay, string VMid, string vmName, int serID, bool node, int indexTerm) {
	/* Fn: ���ڵ㲿�����
	* 	- ��ΪҪ�������˳������������������˳��������������ĵ��ñ��밴��add�����˳��
	*
	* In:
	*	- server: server����
	*	- iDay: ����
	*	- VMid: �����ID
	*	- vmName: ������ͺ�
	*	- serID: �����������ID
	*	- node: ���ڵ㲿��Ľڵ� true��ʾ a�ڵ�
	*
	* Out:
	*	- 0��ʾ�������У�������ڴ���
	*/
	// �жϵ�˫�ڵ����������ʱ�� ��û��ʹ�ô���
	if (info[vmName].nodeStatus) {
		cout << "˫�ڵ����������ָ���ڵ����ͣ�����ʧ��" << endl;
		return 1;
	}

	// �ж���Դ�Ƿ񹻷�
	if (node == true) { // a �ڵ㲿��
		if (server.myServerSet[serID].aIdleCPU < info[vmName].needCPU \
			|| server.myServerSet[serID].aIdleRAM < info[vmName].needRAM) {
			//cout << "a single error" << endl;
			return 2;
		}
		else {
			server.myServerSet[serID].aIdleCPU -= info[vmName].needCPU;
			server.myServerSet[serID].aIdleRAM -= info[vmName].needRAM;
		}
	}
	else {
		if (server.myServerSet[serID].bIdleCPU < info[vmName].needCPU \
			|| server.myServerSet[serID].bIdleRAM < info[vmName].needRAM) {
			cout << "b single error" << endl;
			return 2;
		}
		else {
			server.myServerSet[serID].bIdleCPU -= info[vmName].needCPU;
			server.myServerSet[serID].bIdleRAM -= info[vmName].needRAM;
		}
	}

	sEachWorkingVM oneVM;
	oneVM.vmName = vmName;
	oneVM.serverID = serID;
	oneVM.node = node;
	if (!workingVmSet.count(VMid)) {
		workingVmSet.insert(std::make_pair(VMid, oneVM));
	}
	else {
		cout << "�����id��ͻ��" << endl;
		return 3;
	}

	sDeployItem oneDeploy;
	oneDeploy.serID = serID;
	oneDeploy.isSingle = true;
	oneDeploy.node = node;
	oneDeploy.indexTerm = indexTerm;

	deployRecord[iDay].push_back(oneDeploy);

	return 0;
}

int cVM::deploy(cServer &server, int iDay, string VMid, string vmName, int serID, int indexTerm) {
	/*
	* Fn: ˫�ڵ㲿��
	* 	- ��ΪҪ�������˳������������������˳��������������ĵ��ñ��밴��add�����˳��
	*	- �����ο� ���ڵ㲿�� ����
	*/
	if (!info[vmName].nodeStatus) {
		cout << "���ڵ���������������ڵ㣬����ʧ��" << endl;
		return 1;
	}

	if (server.myServerSet[serID].aIdleCPU<info[vmName].needCPU / 2 \
		|| server.myServerSet[serID].bIdleCPU<info[vmName].needCPU / 2 \
		|| server.myServerSet[serID].aIdleRAM < info[vmName].needRAM / 2 \
		|| server.myServerSet[serID].bIdleRAM < info[vmName].needRAM / 2) {
		cout << "��������Դ����������ʧ��" << endl;
		return 2;
	}
	else {
		server.myServerSet[serID].aIdleCPU -= info[vmName].needCPU / 2;
		server.myServerSet[serID].aIdleRAM -= info[vmName].needRAM / 2;
		server.myServerSet[serID].bIdleCPU -= info[vmName].needCPU / 2;
		server.myServerSet[serID].bIdleRAM -= info[vmName].needRAM / 2;
	}

	sEachWorkingVM oneVM;
	oneVM.vmName = vmName;
	oneVM.serverID = serID;
	if (!workingVmSet.count(VMid)) {
		workingVmSet.insert(std::make_pair(VMid, oneVM));
	}
	else {
		cout << "�����id��ͻ��" << endl;
		return 3;
	}

	sDeployItem oneDeploy;
	oneDeploy.serID = serID;
	oneDeploy.isSingle = false;
	oneDeploy.indexTerm = indexTerm;
	deployRecord[iDay].push_back(oneDeploy);

	return 0;
}

int cVM::deploy(cServer &server, int iDay, string VMid, string vmName, int serID, bool node){
/* Fn: ���ڵ㲿�����
* 	- ��ΪҪ�������˳������������������˳��������������ĵ��ñ��밴��add�����˳��
*
* In: 
*	- server: server����
*	- iDay: ����
*	- VMid: �����ID
*	- vmName: ������ͺ�
*	- serID: �����������ID
*	- node: ���ڵ㲿��Ľڵ� true��ʾ a�ڵ�
*
* Out:
*	- 0��ʾ�������У�������ڴ���
*/
	// �жϵ�˫�ڵ����������ʱ�� ��û��ʹ�ô���
	if(info[vmName].nodeStatus) {
		cout << "˫�ڵ����������ָ���ڵ����ͣ�����ʧ��" << endl;
		return 1;
	}

	// �ж���Դ�Ƿ񹻷�
	if (node==true) { // a �ڵ㲿��
		if (server.myServerSet[serID].aIdleCPU < info[vmName].needCPU \
			|| server.myServerSet[serID].aIdleRAM < info[vmName].needRAM) {
			//cout << "a single error" << endl;
			return 2;
		}
		else {
			server.myServerSet[serID].aIdleCPU -= info[vmName].needCPU;
			server.myServerSet[serID].aIdleRAM -= info[vmName].needRAM;
		}
	}
	else {
		if (server.myServerSet[serID].bIdleCPU < info[vmName].needCPU \
			|| server.myServerSet[serID].bIdleRAM < info[vmName].needRAM) {
			cout << "b single error" << endl;
			return 2;
		}
		else {
			server.myServerSet[serID].bIdleCPU -= info[vmName].needCPU;
			server.myServerSet[serID].bIdleRAM -= info[vmName].needRAM;
		}
	}

	sEachWorkingVM oneVM;
	oneVM.vmName = vmName;
	oneVM.serverID = serID;
	oneVM.node = node;
	if (!workingVmSet.count(VMid)) {
		workingVmSet.insert(std::make_pair(VMid, oneVM));
	}
	else {
		cout << "�����id��ͻ��" << endl;
		return 3;
	}

	sDeployItem oneDeploy;
	oneDeploy.serID = serID;
	oneDeploy.isSingle = true;
	oneDeploy.node = node;

	deployRecord[iDay].push_back(oneDeploy);

	return 0;
}

int cVM::deploy(cServer &server, int iDay, string VMid, string vmName, int serID) {
/*
* Fn: ˫�ڵ㲿��
* 	- ��ΪҪ�������˳������������������˳��������������ĵ��ñ��밴��add�����˳��
*	- �����ο� ���ڵ㲿�� ����
*/
	if(!info[vmName].nodeStatus) {
		cout << "���ڵ���������������ڵ㣬����ʧ��" << endl;
		return 1;
	}

	if (server.myServerSet[serID].aIdleCPU<info[vmName].needCPU/2 \
		|| server.myServerSet[serID].bIdleCPU<info[vmName].needCPU/2 \
		|| server.myServerSet[serID].aIdleRAM < info[vmName].needRAM/2 \
		|| server.myServerSet[serID].bIdleRAM < info[vmName].needRAM/2) {
		cout << "��������Դ����������ʧ��" << endl;
			return 2;
	}
	else {
		server.myServerSet[serID].aIdleCPU -= info[vmName].needCPU/2;
		server.myServerSet[serID].aIdleRAM -= info[vmName].needRAM/2;
		server.myServerSet[serID].bIdleCPU -= info[vmName].needCPU/2;
		server.myServerSet[serID].bIdleRAM -= info[vmName].needRAM/2;
	}

	sEachWorkingVM oneVM;
	oneVM.vmName = vmName;
	oneVM.serverID = serID;
	if (!workingVmSet.count(VMid)) {
		workingVmSet.insert(std::make_pair(VMid, oneVM));
	}
	else {
		cout << "�����id��ͻ��" << endl;
		return 3;
	}

	sDeployItem oneDeploy;
	oneDeploy.serID = serID;
	oneDeploy.isSingle = false;
	deployRecord[iDay].push_back(oneDeploy);

	return 0;
}

void cVM::transfer(cServer &server, int iDay, string VMid, int serID, bool node) {
/*
* Fn: ���ڵ�Ǩ�ƣ������������������������5%����Ҫ���
*/
	string vmName = workingVmSet[VMid].vmName;
	int lastServerID = workingVmSet[VMid].serverID;
	bool lastServerNode = workingVmSet[VMid].node;
	int occupyCPU = info[vmName].needCPU;
	int occupyRAM = info[vmName].needRAM;

	// �ָ�ǰһ������������Դ
	if (lastServerNode==true) { // A node
		server.myServerSet[lastServerID].aIdleCPU += occupyCPU;
		server.myServerSet[lastServerID].aIdleRAM += occupyRAM;
	}
	else {
		server.myServerSet[lastServerID].bIdleCPU += occupyCPU;
		server.myServerSet[lastServerID].bIdleRAM += occupyRAM;
	}

	// ռ���·�������Դ
	if (node==true) {
		server.myServerSet[serID].aIdleCPU -= occupyCPU;
		server.myServerSet[serID].aIdleRAM -= occupyRAM;
	}
	else {
		server.myServerSet[serID].bIdleCPU -= occupyCPU;
		server.myServerSet[serID].bIdleRAM -= occupyRAM;
	}

	// Ǩ����Ŀ����
	sTransVmItem oneTrans;
	oneTrans.vmID = VMid;
	oneTrans.serverID = serID;
	oneTrans.node = node;
	oneTrans.isSingle = true;
	transVmRecord[iDay].push_back(oneTrans);
}

void cVM::transfer(cServer &server, int iDay, string VMid, int serID) {
/*
* Fn: ˫�ڵ�Ǩ�ƣ������������������
*/
	string vmName = workingVmSet[VMid].vmName;
	int lastServerID = workingVmSet[VMid].serverID;
	int occupyCPU = info[vmName].needCPU;
	int occupyRAM = info[vmName].needRAM;

	// �ָ�ǰһ������������Դ
	server.myServerSet[lastServerID].aIdleCPU += occupyCPU/2;
	server.myServerSet[lastServerID].aIdleRAM += occupyRAM/2;
	server.myServerSet[lastServerID].bIdleCPU += occupyCPU/2;
	server.myServerSet[lastServerID].bIdleRAM += occupyRAM/2;

	// ռ���·�������Դ
	server.myServerSet[serID].aIdleCPU -= occupyCPU/2;
	server.myServerSet[serID].aIdleRAM -= occupyRAM/2;
	server.myServerSet[serID].bIdleCPU -= occupyCPU/2;
	server.myServerSet[serID].bIdleRAM -= occupyRAM/2;

	// Ǩ����Ŀ����
	sTransVmItem oneTrans;
	oneTrans.vmID = VMid;
	oneTrans.serverID = serID;
	oneTrans.isSingle = false;
	transVmRecord[iDay].push_back(oneTrans);
}

bool cVM::isDouble(string vmName) {
	return info[vmName].nodeStatus;
}

int cVM::reqCPU(string vmName) {
	return info[vmName].needCPU;
}

int cVM::reqRAM(string vmName) {
	return info[vmName].needRAM;
}

void cVM::deleteVM(string vmID, cServer& server) {
	string vmName = workingVmSet[vmID].vmName;
	bool doubleStatus = isDouble(vmName);
	int serID = workingVmSet[vmID].serverID;
	int reqCPUs = reqCPU(vmName);
	int reqRAMs = reqRAM(vmName);

	// �ָ���������Դ
	if (doubleStatus) {
		server.myServerSet[serID].aIdleCPU += reqCPUs/2;
		server.myServerSet[serID].bIdleCPU += reqCPUs/2;
		server.myServerSet[serID].aIdleRAM += reqRAMs/2;
		server.myServerSet[serID].bIdleRAM += reqRAMs/2;
	}
	else {
		if (workingVmSet[vmID].node) { // A
			server.myServerSet[serID].aIdleCPU += reqCPUs;
			server.myServerSet[serID].aIdleRAM += reqRAMs;
		}
		else { // B
			server.myServerSet[serID].bIdleCPU += reqCPUs;
			server.myServerSet[serID].bIdleRAM += reqRAMs;
		}
	}
}

void cVM::predp(string vmID, bool isNew, int serID, string vmName, cServer &server, bool node) {
/* Fn: Ԥ���� ���ڵ�
*/
	int reqCPU = info[vmName].needCPU;
	int reqRAM = info[vmName].needRAM;

	// ������Դ (preSvSet & mySerCopy)
	if (isNew) {
		if (node) {
			server.preSvSet[serID].aIdleCPU -= reqCPU;
			server.preSvSet[serID].aIdleRAM -= reqRAM;
		}
		else {
			server.preSvSet[serID].bIdleCPU -= reqCPU;
			server.preSvSet[serID].bIdleRAM -= reqRAM;
		}
	}
	else {
		if (node) {
			server.mySerCopy[serID].aIdleCPU -= reqCPU;
			server.mySerCopy[serID].aIdleRAM -= reqRAM;
		}
		else {
			server.mySerCopy[serID].bIdleCPU -= reqCPU;
			server.mySerCopy[serID].bIdleRAM -= reqRAM;
		}
	}

	// �������ݽṹ
	sPreDeployItem onePreDeploy;

	onePreDeploy.isNew = isNew;
	onePreDeploy.serID = serID;
	onePreDeploy.node = node;
	onePreDeploy.vmName = vmName;

	pair<string, sPreDeployItem> res(vmID, onePreDeploy);
	preDeploy.insert(res);
}

void cVM::predp(string vmID, bool isNew, int serID, string vmName, cServer &server) {
/* Fn: Ԥ���� ˫�ڵ�
*/
	int reqCPU = info[vmName].needCPU;
	int reqRAM = info[vmName].needRAM;
	
	if (isNew) {
		server.preSvSet[serID].aIdleCPU -= reqCPU/2;
		server.preSvSet[serID].bIdleCPU -= reqCPU/2;
		server.preSvSet[serID].aIdleRAM -= reqRAM/2;
		server.preSvSet[serID].bIdleRAM -= reqRAM/2;
	}
	else {
		server.mySerCopy[serID].aIdleCPU -= reqCPU/2;
		server.mySerCopy[serID].bIdleCPU -= reqCPU/2;
		server.mySerCopy[serID].aIdleRAM -= reqRAM/2;
		server.mySerCopy[serID].bIdleRAM -= reqRAM/2;
	}

	sPreDeployItem onePreDeploy;

	onePreDeploy.isNew = isNew;
	onePreDeploy.serID = serID;
	onePreDeploy.vmName = vmName;

	pair<string, sPreDeployItem> res(vmID, onePreDeploy);
	preDeploy.insert(res);
}

void cVM::preDltVM(cServer &server, string vmID) {
	preDel.push_back(vmID);
}

int cVM::postDpWithDel(cServer &server, const cRequests &request, int iDay) {
	int postid;
	int bugID;
	string vmID;
	string vmName;
	bool isdouble;
	bool isNew;
	int preid;
	bool node;

	for (int iTerm=0; iTerm<request.numEachDay[iDay]; iTerm++) {
		if (request.info[iDay][iTerm].type) { //
			vmID = request.info[iDay][iTerm].vmID;
			vmName = preDeploy[vmID].vmName;
			isdouble = isDouble(vmName);
			isNew = preDeploy[vmID].isNew;
			preid = preDeploy[vmID].serID;
			node = preDeploy[vmID].node;

			// id ӳ��
			if (isNew)
				postid = server.idMap[preid];
			else
				postid = preid;

			if (isdouble)
				bugID = deploy(server, iDay, vmID, vmName, postid);
			else
				bugID = deploy(server, iDay, vmID, vmName, postid, node);

			if (bugID) {
				cout << "����ʧ��" << bugID << endl;
				return -1;
			}
		}
		else {
			vmID = request.info[iDay][iTerm].vmID;
			deleteVM(vmID, server);
		}
	}

	server.mySerCopy = server.myServerSet;
	server.idMap.clear();

	return 0;
}