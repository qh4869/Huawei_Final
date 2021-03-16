#include "io.h"
#include "Server.h"
#include "VM.h"
#include "Request.h"
#include "firstFit.h"
#include "graphFit.h"
#include <iostream>
#include "globalHeader.h" // ȫ�ֳ���
using namespace std;

int main()
{
	cServer server;
	cVM VM;
	cRequests request;
	void process(cServer &server, cVM &VM, const cRequests &request);

	// ����
	tie(server, VM, request) = dataIn("training-1.txt");

	// ����Ԥ����
	server.alpha = ALPHA;
	server.rankServerByPrice(); // ���ռ۸����� Ȩ��Ϊalpha

	// ����Ǩ�ƣ�����
	process(server, VM, request);

#ifndef LOCAL
	// ���
	dataOut(server, VM, request);
#endif
	//dataOut(server, VM, request);
	//cout << server.price << endl;
	system("pause");
	return 0;
}

void process(cServer &server, cVM &VM, const cRequests &request) {
	//firstFit(server, VM, request);
	graphFit(server, VM, request);
}