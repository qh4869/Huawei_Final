#pragma once
#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <tuple>
#include <utility>
#include "Request.h"
#include "globalHeader.h"
using namespace std;

struct sServerItem {
	int totalCPU;
	int totalRAM;
	int hardCost; // hardware cost
	int energyCost; // energy Cost per day
	// cyt
	string serName;  // ����������
	int buyID;  // ���ڼ�¼�ѹ����������ID
	bool node;   // true : a �ڵ�
};

struct sMyEachServer {
	string serName;
	int aIdleCPU; // ����node ������Դ
	int bIdleCPU;
	int aIdleRAM;
	int bIdleRAM;
};

class cServer {
public:

	int serverTypeNum;
	unordered_map<string, sServerItem> info; // �������ͺ� -> ������Ϣ
	int alpha; // �۸��Ȩ����
	vector<pair<string, int>> priceOrder; // ���з������۸��С�����������д��� <���������ͺ�, ��Ȩ�۸�>
	vector<sMyEachServer> myServerSet; // ��ǰ�Ѿ�����ķ����� ������id����vector��index
	vector<sMyEachServer> preSvSet; // Ԥ���������vector�������뵽myServerSet�У�cVM::buy����������
	vector<sMyEachServer> mySerCopy; // myServerSet�ĸ���������Ԥ�����Ԥ�����ڼ䣬cVM::postDpWithDel������ͬ��
	unordered_map<int, int> idMap; // �¾�id��ӳ���ϵ

	vector<int> newServerNum; // �������: ÿ�칺��ķ����������� 
							  // vector<unordered_map<string, int>> buyRecord; // �������: ÿ�� �����ÿ�ַ����� �����¼ (hash: �������ͺ�->��Ŀ)
	//vector<vector<pair<string, int>>> buyRecord; // ÿ�� �����������¼
	vector<unordered_map<string, int>> buyRecord; // ÿ�� �����ÿ�ַ����� �����¼ ������� (hash: �������ͺ�->��Ŀ)

												 // ���ռ۸�����
	static bool mycomp(pair<string, int> i, pair<string, int> j);
	void rankServerByPrice();
	void rankServerByPrice(bool flag);   // alpha��ʾȨ�أ�flag��ʾ�Ƿ��ǵ�һ�����ɣ����ǵĻ����Խ���һ��㸴�Ӷ�

	// ����ĳ̨�������Ƿ�Ϊ����״̬
	bool isOpen(int serID);

	int prePurchase(string name); // Ԥ���������
	//int purchase(string serName, int iDay); // ��������� id���չ���˳����䣨���ܲ��������Ҫ��
	void purchase(string serName, int iDay); // ���������
	void buy(int iDay);// Ԥ����תΪ��ʽ����
	
	// First Fit����0�ŷ�������ʼ��һ̨��װ��ȥ�ķ�����ID, �Ҳ�������-1��
	pair<bool, int> firstFitDouble(int reqCPU, int reqRAM); // ˫�ڵ㲿��
	std::tuple<pair<bool, int>, bool> firstFitSingle(int reqCPU, int reqRAM); // ���ڵ㲿��
	// ѡ������������ĳ�������ķ������б����˳�����ܹ����������Ҫ�� ��������˵ķ����� (�������Ҳ���)
	string chooseSer(int reqCPU, int reqRAM, bool isDoubleNode);

	// CYT 
	int price;
	// ��������Դ��С�������򣬵�һ��int��ʾ�ѹ����������id��������myServerSetӳ�䣬�ڶ���ʾ��Դ��
	vector<pair<int, int>> resourceOrder;     
	vector<int> dayServerNum;   // ÿ�����ӵķ���������
	unordered_map<int, int> realToVir; // ��ʵ��ID��Ӧ�����ID
	unordered_map<int, int> virToReal; //�����ID��Ӧ��ʵ��ID

	static bool rescomp(pair<int, int> i, pair<int, int> j);    // ������Դ������
	bool rescomp_equal(pair<int, int> i, pair<int, int> j);
	void rankServerByResource(pair<int, int> newOne);    // ����Դ��������
	void updateRank(int pos, bool flag);   // ������µ�λ��
};
