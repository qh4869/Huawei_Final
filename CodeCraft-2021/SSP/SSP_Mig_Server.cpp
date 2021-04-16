#include "SSP_Mig_Server.h"

int cSSP_Mig_Server::purchase(string name, int iDay) {
/* In:
*	- name: 服务器型号
* 	- iDay: 第几天
*
* Out:
*	- 购买的服务器ID
*/
	/*add into myServerSet*/
	sMyEachServer oneServer;

	oneServer.serName = name;
	oneServer.aIdleCPU = info[name].totalCPU / 2;
	oneServer.bIdleCPU = info[name].totalCPU / 2;
	oneServer.aIdleRAM = info[name].totalRAM / 2;
	oneServer.bIdleRAM = info[name].totalRAM / 2;
	myServerSet.push_back(oneServer);

	/*记录购买日期*/
	purchaseDate.insert({myServerSet.size() - 1, iDay});

	/*add into serverVMSet-empty*/
	unordered_map<string, int> vmSet;
	serverVMSet.push_back(vmSet);

	/*add into buyRecord*/
	if (!buyRecord[iDay].count(name)) { // 当天没买过这个类型
		buyRecord[iDay].insert(make_pair(name, 1));
	}
	else { // 当天买过这个类型
		buyRecord[iDay][name]++;
	}

	return myServerSet.size() - 1;
}

bool cyt::mycompID(pair<int, int> i, pair<int, int> j) {
	return i.second <= j.second;
}

// 对服务器里的虚拟机数量进行排序
void cSSP_Mig_Server::updatVmSourceOrder(int needCPU, int needRAM, int serID, bool flag) {  // true : add , false : delete

	int pos;   // serID对应的服务器在vmNumOrder中的位置
	if (vmSourceOrder.size() == 0) {    // 初始化
		int value = needCPU + needRAM;
		vmSourceOrder.push_back(make_pair(serID, value));   // 最开始直接初始化
		return;
	}
	else if (serID > (int)vmSourceOrder.size() - 1) {   // 新加入的服务器
		if (!flag) {
			cout << "sort error" << endl;
		}
		vmSourceOrder.insert(vmSourceOrder.begin(), make_pair(serID, 0));   // 将新加入的插进第一个位置，初始值为0
		pos = 0;
	}
	else {
		for (int i = 0; i < (int)vmSourceOrder.size(); i++) {
			if (vmSourceOrder[i].first == serID) {   // 找到对应的位置
				pos = i;
				break;
			}
		}
	}


	if (!flag && pos == 0) {  // 删除并且处于第一位，则不需要移动
		vmSourceOrder[pos].second -= (needCPU + needRAM);   // 减去删除的虚拟机资源
	}
	else {
		pair<int, int> temp = vmSourceOrder[pos];    // 取出该变量
		if (flag) {    // true : add
			temp.second += (needCPU + needRAM);
		}
		else {   // false : delete
			temp.second -= (needCPU + needRAM);
		}
		auto ite = vmSourceOrder.begin();
		if (pos - 1 < 0 || cyt::mycompID(vmSourceOrder[pos - 1], temp)) {  // pos - 1 < 0表示是第一个元素
			if (pos != (int)vmSourceOrder.size() - 1) {   // 不是最后一个元素
				for (int i = pos + 1; i < (int)vmSourceOrder.size(); i++) {
					if (i == (int)vmSourceOrder.size() - 1 && cyt::mycompID(vmSourceOrder[i], temp)) {  // 最后一个元素还是小
						vmSourceOrder.erase(ite + pos);
						vmSourceOrder.push_back(temp);
						break;
					}
					if (cyt::mycompID(temp, vmSourceOrder[i])) {   // 往下找到了比自己大的，要插在这个比自己大的前面
						vmSourceOrder.erase(ite + pos);   // 删除该元素
						vmSourceOrder.insert(vmSourceOrder.begin() + i - 1, temp);   // 在指定的位置插入元素
						break;   // 退出
					}
				}
			}
			else {
				vmSourceOrder[pos] = temp;   // 赋予新值
			}
		}
		else {
			for (int i = pos - 1; i >= 0; i--) {   // 走到这里说明了pos - 1 > 0
				if (i == 0 && cyt::mycompID(temp, vmSourceOrder[i])) {
					vmSourceOrder.erase(ite + pos);
					vmSourceOrder.insert(vmSourceOrder.begin() + i, temp);
					break;
				}
				if (cyt::mycompID(vmSourceOrder[i], temp)) {
					vmSourceOrder.erase(ite + pos);
					vmSourceOrder.insert(vmSourceOrder.begin() + i + 1, temp);
					break;
				}
			}
		}
	}

}

void cSSP_Mig_Server::updatVmTarOrder(int needCPUa, int needRAMa, int needCPUb, int needRAMb, int serID, bool flag) {  // true : add , false : delete

	string serName = myServerSet[serID].serName;
	int aIdleCPU = myServerSet[serID].aIdleCPU;
	int aIdleRAM = myServerSet[serID].aIdleRAM;
	int bIdleCPU = myServerSet[serID].bIdleCPU;
	int bIdleRAM = myServerSet[serID].bIdleRAM;
	int alastCPU, alastRAM, blastCPU, blastRAM;

	/*加入新结点*/
	if (isOpen(serID)) // 如果服务器已清空，就不加入VmTarOrder列表里，直接不作为迁入服务器备选
		vmTarOrder[aIdleCPU][aIdleRAM][bIdleCPU][bIdleRAM].push_back(serID);

	if (flag) {
		alastCPU = aIdleCPU + needCPUa;
		alastRAM = aIdleRAM + needRAMa;
		blastCPU = bIdleCPU + needCPUb;
		blastRAM = bIdleRAM + needRAMb;
	}
	else {
		alastCPU = aIdleCPU - needCPUa;
		alastRAM = aIdleRAM - needRAMa;
		blastCPU = bIdleCPU - needCPUb;
		blastRAM = bIdleRAM - needRAMb;
	}

	/*删除旧节点*/
	if (vmTarOrder.count(alastCPU) && vmTarOrder[alastCPU].count(alastRAM) \
		&& vmTarOrder[alastCPU][alastRAM].count(blastCPU) && vmTarOrder[alastCPU][alastRAM][blastCPU].count(blastRAM)) {
		vector<int> &serSet = vmTarOrder[alastCPU][alastRAM][blastCPU][blastRAM];
		for (auto it = serSet.begin(); it != serSet.end(); it++) {
			if (*it == serID) {
				serSet.erase(it);
				break;
			}
		}
		if (serSet.empty()) {
			vmTarOrder[alastCPU][alastRAM][blastCPU].erase(blastRAM);
			if (vmTarOrder[alastCPU][alastRAM][blastCPU].empty()) {
				vmTarOrder[alastCPU][alastRAM].erase(blastCPU);
				if (vmTarOrder[alastCPU][alastRAM].empty()) {
					vmTarOrder[alastCPU].erase(alastRAM);
					if (vmTarOrder[alastCPU].empty()) {
						vmTarOrder.erase(alastCPU);
					}
				}
			}
		}
	}
}
