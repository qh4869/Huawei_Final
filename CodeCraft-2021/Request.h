#pragma once
#include <string>
#include <vector>
#include "globalHeader.h"
using namespace std;

struct sRequestItem {
    bool type; // operation type, true��ʾadd false��ʾdelete
    string vmName; // delete���������ֵΪ��
    string vmID;
};

class cRequests {
public:
    int dayNum; // ����
    vector<int> numEachDay; // ÿ�����������
    vector<vector<sRequestItem>> info; // ĳһ��� ĳ�� ����
};