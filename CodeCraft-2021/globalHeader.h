#pragma once

// #define LOCAL // 上传把这行注释掉
// CMakeList -> release

#ifdef LOCAL
#include<time.h>
#endif

const int ALPHA = 400; // 价格加权参数
const int KSSIZE = 3; // 背包算法的vm的分组最大个数
