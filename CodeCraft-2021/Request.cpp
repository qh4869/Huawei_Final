#include "Request.h"

void cRequest::getNumEachDay() {
	/* Fn: ����ÿ���add/delete��������, ���װ��numAddEachDay��numDelEachDay
	*/
	// �������йص�vector��ʼ��
	numAddEachDay.resize(dayNum);
	numDelEachDay.resize(dayNum);

	for (int iDay = 0; iDay < dayNum; iDay++) {
		//��ʼ��ÿ���add��delete��������
		numAddEachDay[iDay] = 0;
		numDelEachDay[iDay] = 0;

		for (int iTerm = 0; iTerm < numEachDay[iDay]; iTerm++) {
			//���
			if (info[iDay][iTerm].type) { // add ����
				numAddEachDay[iDay]++;
			}
			else { // delete ����
				numDelEachDay[iDay]++;
			}
		}
	}
}

void cRequest::getVarAdd() {
	/* Fn: ���Add����ķ�����װ�� varAdd
	*/
	varAdd = 0;
	double meanAdd = 0;
	for (int iDay = 0; iDay < dayNum; iDay++) {
		meanAdd += numAddEachDay[iDay];
	}
	meanAdd /= dayNum;

	for (int iDay = 0; iDay < dayNum; iDay++) {
		varAdd += (numAddEachDay[iDay] - meanAdd) * (numAddEachDay[iDay] - meanAdd);
		varAdd /= (dayNum - 1);
	}
}

void cRequest::getVarDel() {
	/* Fn: ���delete����ķ�����װ�� varDel
	*/
	varDel = 0;
	double meanDel = 0;
	for (int iDay = 0; iDay < dayNum; iDay++) {
		meanDel += numDelEachDay[iDay];
	}
	meanDel /= dayNum;

	for (int iDay = 0; iDay < dayNum; iDay++) {
		varDel += (numDelEachDay[iDay] - meanDel) * (numDelEachDay[iDay] - meanDel);
		varDel /= (dayNum - 1);
	}
}