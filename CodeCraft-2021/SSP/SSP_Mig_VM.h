#pragma once
#include "VM.h"
#include "SSP_Mig_Server.h"

struct sPosGoal {
	int goal; // 如果没有任何一台能够装进这个vm，goal=-1
	int serID;
	bool node;
};

class cSSP_Mig_VM : public cVM {
public:

	/*重载，添加迁移服务器排序更新，以及服务器有哪些VM的记录*/
	void deploy(cSSP_Mig_Server &server, int iDay, string VMid, string vmName, int serID, bool node);
	void deploy(cSSP_Mig_Server &server, int iDay, string VMid, string vmName, int serID);
	void deleteVM(string vmID, cSSP_Mig_Server& server);
	void transfer(cSSP_Mig_Server &server, int iDay, string VMid, int serID, bool node);
	void transfer(cSSP_Mig_Server &server, int iDay, string VMid, int serID);

	/*记录某种型号当前的最优位置，不摘除自己。防止同一种型号vm遍历迁入ser时候，执行相同的计算*/
	unordered_map<string, sPosGoal> saveGoal;
	/*如果某台虚拟机总是找不到合适的迁入服务器，那就加入到这个集合，暂时永不解除封印*/
	unordered_map<string, int> stopSetCnt; // <vmID, cnt>
	bool isLocked(string vmID);
	void addLock(string vmID);

	/*记录del的次数，用于清理stopSetCnt*/
	int delCnt = 0;

	/*自定义参数*/
	double ratio = 1;
	int migFind = 1000;
	int maxIter = 1;
	int fitThreshold = 1000;
	int stopTimes = 3;
	int delayTimes = -1;
	int delCntMax = -1;
};