#pragma once
#include "VM.h"
#include "SSP_Mig_Server.h"

class cSSP_Mig_VM : public cVM {
public:

	/*重载，添加迁移服务器排序更新，以及服务器有哪些VM的记录*/
	void deploy(cSSP_Mig_Server &server, int iDay, string VMid, string vmName, int serID, bool node);
	void deploy(cSSP_Mig_Server &server, int iDay, string VMid, string vmName, int serID);
	void deleteVM(string vmID, cSSP_Mig_Server& server);
	void transfer(cSSP_Mig_Server &server, int iDay, string VMid, int serID, bool node);
	void transfer(cSSP_Mig_Server &server, int iDay, string VMid, int serID);
};