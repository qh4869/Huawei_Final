#include "io.h"
using namespace std;

std::tuple<cServer, cVM, cRequests> dataIn(string fileName) {
	/*
	* Fn: ��������
	*
	* In:
	*   - fileName: txt�ļ���
	*
	*/
#ifdef LOCAL
	ifstream fin;
	fin.open(fileName.c_str());
	cin.rdbuf(fin.rdbuf()); // �ض���
#endif

	string inputStr;
	cServer server;
	cVM VM;
	cRequests request;

	// ������������
	cin >> inputStr;
	server.serverTypeNum = atoi(inputStr.c_str()); // string to int
												   // ������ÿ����Ϣ
	for (int iTerm = 0; iTerm<server.serverTypeNum; iTerm++) {
		// server name
		string serName;
		cin >> inputStr;
		inputStr.erase(inputStr.begin()); // ɾ��������
		inputStr.erase(inputStr.end() - 1); // ����
		serName = inputStr;

		// CYT add serName
		server.info[serName].serName = inputStr;

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

	// ���������
	cin >> inputStr;
	VM.vmTypeNum = atoi(inputStr.c_str());
	// �������ÿ����Ϣ
	for (int iTerm = 0; iTerm<VM.vmTypeNum; iTerm++) {
		// vm name
		string vmName;
		cin >> inputStr;
		inputStr.erase(inputStr.begin()); // ɾ��������
		inputStr.erase(inputStr.end() - 1); // ����
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

	// ������������
	cin >> inputStr;
	request.dayNum = atoi(inputStr.c_str());
	// �������йص�vector��ʼ��
	request.numEachDay.resize(request.dayNum);
	request.info.resize(request.dayNum);
	server.newServerNum.resize(request.dayNum);
	server.buyRecord.resize(request.dayNum);
	VM.transVmRecord.resize(request.dayNum);
	VM.deployRecord.resize(request.dayNum);
	// ÿ�������
	for (int iDay = 0; iDay<request.dayNum; iDay++) {
		// ÿ���������Ŀ
		cin >> inputStr;
		request.numEachDay[iDay] = atoi(inputStr.c_str());
		request.info[iDay].resize(request.numEachDay[iDay]);

		// ÿ����������Ϣ
		for (int iTerm = 0; iTerm<request.numEachDay[iDay]; iTerm++) {
			// ��������
			cin >> inputStr;
			inputStr.erase(inputStr.begin()); // ɾ��������
			inputStr.erase(inputStr.end() - 1); // ����
			request.info[iDay][iTerm].type = (inputStr == "add") ? true : false;

			// ������������ͺ�
			if (request.info[iDay][iTerm].type) { // add����������ͺţ�del�������ͺ�
				cin >> inputStr;
				inputStr.erase(inputStr.end() - 1);
				request.info[iDay][iTerm].vmName = inputStr;
			}

			// ���������ID
			cin >> inputStr;
			inputStr.erase(inputStr.end() - 1);
			request.info[iDay][iTerm].vmID = inputStr;
		}
	}

	// cout << server.serverTypeNum << endl;

	return make_tuple(server, VM, request);
}

void dataOut(cServer& server, cVM& VM, const cRequests& request) {
	/*
	* Fn: ������������׼���
	*
	* In:
	*
	*/
	for (int iDay = 0; iDay < request.dayNum; iDay++) {
		//�����purchase,Q) , Q��ʾ���칺��ķ�����������
		cout << "(" << "purchase" << "," << server.newServerNum[iDay] << ")" << endl;//Ҳ������ server.buyRecord[iDay].size()
																					 //���(�������ͺţ�����������
		//for (auto ite = server.buyRecord[iDay].begin(); ite != server.buyRecord[iDay].end(); ite++) { //������
		//	cout << "(" << ite->first << "," << ite->second << ")" << endl;
		//}
		//////////////////////////////////////////////////////////////////////////////////////////
		unordered_map<string, int> record = server.buyRecord[iDay];
		for (auto ite = record.begin(); ite != record.end(); ite++) {
			cout << "(" << ite->first << "," << ite->second << ")" << endl;
		}
		/////////////////////////////////////////////////////////////////////////////////////////
		//�����migration,W) , W��ʾ����ҪǨ�Ƶ����������
		cout << "(" << "migration," << VM.transVmRecord[iDay].size() << ")" << endl;
		//�ֱ����W̨�������Ǩ��·��
		for (auto ite = VM.transVmRecord[iDay].begin(); ite != VM.transVmRecord[iDay].end(); ite++) { //������
			if (ite->isSingle) {
				if (ite->node)
					cout << "(" << ite->vmID << "," << ite->serverID << "," << "A" << ")" << endl;
				else
					cout << "(" << ite->vmID << "," << ite->serverID << "," << "B" << ")" << endl;
			}
			else
				cout << "(" << ite->vmID << "," << ite->serverID << ")" << endl;
		}
		//�������ÿ������������¼ 
		for (auto ite = VM.realDeployRecord[iDay].begin(); ite != VM.realDeployRecord[iDay].end(); ite++) { //������
			if (ite->serID < 0) {
				continue;
			}
			if (ite->isSingle) {
				if (ite->node)
					cout << "(" << ite->serID << "," << "A" << ")" << endl;
				else
					cout << "(" << ite->serID << "," << "B" << ")" << endl;
			}
			else
				cout << "(" << ite->serID << ")" << endl;
		}
		//for (auto ite = VM.deployRecord[iDay].begin(); ite != VM.deployRecord[iDay].end(); ite++) { //������
		//	if (ite->isSingle) {
		//		if (ite->node)
		//			cout << "(" << ite->serID << "," << "A" << ")" << endl;
		//		else
		//			cout << "(" << ite->serID << "," << "B" << ")" << endl;
		//	}
		//	else
		//		cout << "(" << ite->serID << ")" << endl;
		//}
		// cout << "day end" << endl;
	}
}