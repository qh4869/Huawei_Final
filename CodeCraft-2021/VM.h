#pragma once
#include <unordered_map>
#include "Server.h"
#include <iostream>
#include "globalHeader.h"
using namespace std;

struct sVmItem { // hash map �� value����
    int needCPU;
    int needRAM;
    bool nodeStatus; // true��ʾ˫�ڵ�
};

struct sEachWorkingVM {
	string vmName;
	int serverID;
	bool node; // true ��ʾa�ڵ� false ��ʾb�ڵ�
};

struct sTransVmItem {
	string vmID;
	int serverID;
    bool isSingle;
	bool node; // ˫�ڵ��������������û������
};

struct sDeployItem {
	int serID;
	bool isSingle;
	bool node; // true a�ڵ�
	int indexTerm;
};

struct sPreDeployItem {
    bool isNew; // �Ƿ����������
    int serID;
    bool node; // true: node a
    string vmName;
};

class cVM {
public:
    int vmTypeNum;
    unordered_map<string, sVmItem> info; // hash map: type->info
    unordered_map<string, sEachWorkingVM> workingVmSet; // �����е������ ��ϣ�� �����ID -> ���������Ϣ
    unordered_map<string ,sPreDeployItem> preDeploy; // Ԥ���� vmID->����
    vector<string> preDel; // Ԥɾ�� vmID

    vector<vector<sTransVmItem>> transVmRecord; // ÿ�� ÿ�� Ǩ�Ƽ�¼ �������
    vector<vector<sDeployItem>> deployRecord; // ÿ�� ÿ�� ����������¼ �������
	vector<vector<sDeployItem>> realDeployRecord; // ÿ�� ÿ�� ����������¼ �������

    // ���ڵ�Ԥ����
    void predp(string vmID, bool isNew, int serID, string vmName, cServer &server, bool node);
    // ˫�ڵ�Ԥ����
    void predp(string vmID, bool isNew, int serID, string vmName, cServer &server);

    // Ԥɾ�������
    void preDltVM(cServer &server, string vmID);

    // ����
    int postDpWithDel(cServer &server, const cRequests &request, int iDay);

    // ���ڵ㲿��
    int deploy(cServer &server, int iDay, string VMid, string vmName, int serID, bool node);
    // ˫�ڵ㲿��
    int deploy(cServer &server, int iDay, string VMid, string vmName, int serID);

    // delete VM
    void deleteVM(string vmID, cServer& server);

    // ���ڵ������Ǩ��
    void transfer(cServer &server, int iDay, string VMid, int serID, bool node);
    // ˫�ڵ������Ǩ��
    void transfer(cServer &server, int iDay, string VMid, int serID);

    // ������������֣������Ƿ��� ˫�ڵ㲿��
    bool isDouble(string vmName);
    // ������������֣�������ҪCPU
    int reqCPU(string vmName);
    // ������������֣�������ҪRAM
    int reqRAM(string vmName);

	// CYT
	int deploy(cServer &server, int iDay, string VMid, string vmName, int serID, bool node, int indexTerm);
	int deploy(cServer &server, int iDay, string VMid, string vmName, int serID, int indexTerm);
};
