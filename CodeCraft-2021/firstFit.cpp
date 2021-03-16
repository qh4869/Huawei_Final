#include "firstFit.h"

void firstFit(cServer &server, cVM &VM, const cRequests &request) {
	/* Fn: first fit�㷨����͹��򣬲�Ǩ��
	*/
#ifdef LOCAL
	// ���Ƴɱ�����
	int engCostStas = 0; // ���㹦�ĳɱ�
	int hardCostStas = 0;
	string costName;
#endif

	// ��ʼ������
	int serId;
	pair<bool, int> serPosId;
	bool serNode;
	string serName;
	string vmName;
	string vmID;
	bool vmIsDouble;
	int vmReqCPU;
	int vmReqRAM;
	int bugID;

	for (int iDay = 0; iDay<request.dayNum; iDay++) {
		cout << iDay << endl;
		for (int iTerm = 0; iTerm<request.numEachDay[iDay]; iTerm++) {
			// Ԥ�����Ԥ����			
			if (request.info[iDay][iTerm].type) { // add ����
												  // ��ȡ�������Ϣ
				vmName = request.info[iDay][iTerm].vmName;
				vmID = request.info[iDay][iTerm].vmID;
				vmIsDouble = VM.isDouble(vmName);
				vmReqCPU = VM.reqCPU(vmName);
				vmReqRAM = VM.reqRAM(vmName);

				// �������еķ����� ��0�ŷ�������ʼ
				if (vmIsDouble) { // ˫�ڵ�
					serPosId = server.firstFitDouble(vmReqCPU, vmReqRAM);
				}
				else {
					tie(serPosId, serNode) = server.firstFitSingle(vmReqCPU, vmReqRAM);
				}

				// �����������Ԥ�����Ԥ����
				if (serPosId.second != -1) { // ���ù��򣬵�Ҳ�п����ǽ������
					if (vmIsDouble)
						VM.predp(vmID, serPosId.first, serPosId.second, vmName, server);
					else
						VM.predp(vmID, serPosId.first, serPosId.second, vmName, server, serNode);
				}
				else { // ��Ҫ����
					serName = server.chooseSer(vmReqCPU, vmReqRAM, vmIsDouble);
					serId = server.prePurchase(serName);
					if (vmIsDouble)
						VM.predp(vmID, true, serId, vmName, server);
					else
						VM.predp(vmID, true, serId, vmName, server, true);
				}
			}
			else { // delete ����
				vmID = request.info[iDay][iTerm].vmID;
				VM.preDltVM(server, vmID);
			}
		}
		// ��ʽ����
		server.buy(iDay);
		// ��˳��������
		bugID = VM.postDpWithDel(server, request, iDay);
		if (bugID) {
			cout << "����ʧ��" << bugID << endl;
			return;
		}
#ifdef LOCAL
		// һ�����ͳ�ƹ��ĳɱ�
		for (int iServer = 0; iServer<(int)server.myServerSet.size(); iServer++) {
			if (server.isOpen(iServer)) {
				costName = server.myServerSet[iServer].serName;
				engCostStas += server.info[costName].energyCost;
			}
		}
#endif
	}
#ifdef LOCAL
	// ������������ܳɱ�
	for (int iServer = 0; iServer<(int)server.myServerSet.size(); iServer++) {
		costName = server.myServerSet[iServer].serName;
		hardCostStas += server.info[costName].hardCost;
	}
	cout << "�ɱ�: " << engCostStas + hardCostStas << endl;
#endif
}