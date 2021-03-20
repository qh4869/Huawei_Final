#pragma once

#define LOCAL // 上传把这行注释掉
#define INT_MAX 1000000000
// CMakeList -> release

const int ALPHA = 400; // 价格加权参数

struct sVmItem { // hash map 的 value部分
	int needCPU;
	int needRAM;
	bool nodeStatus; // true表示双节点
};