#include "io.h"
#include "SSP_Mig_Server.h"
#include "SSP_Mig_VM.h"
#include "ssp.h"
// #include "FF_Server.h"
// #include "firstFit.h"
#include "Request.h"
#include <iostream>
#include "globalHeader.h" 
#ifdef LOCAL
#include<time.h>
#endif

using namespace std;

#ifdef LOCAL
clock_t TIMEstart, TIMEend;
#endif
int main()
{
#ifdef LOCAL
	TIMEstart = clock();
#endif
	cSSP_Mig_Server server;
	cSSP_Mig_VM VM;
	cRequests request;

	// 输入
	dataIn("../CodeCraft-2021/training-1.txt", server, VM, request);

	// 购买，迁移，部署
	ssp(server, VM, request);

	// id map
	server.idMapping(); 

	// 输出
	dataOut(server, VM, request);

	return 0;
}
