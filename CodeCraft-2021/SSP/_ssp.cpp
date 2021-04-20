#include "ssp.h"

map<int, map<int, map<int, map<int, vector<int>>>>>::iterator \
greaterEqu1(map<int, map<int, map<int, map<int, vector<int>>>>> &set, int val) {
	map<int, map<int, map<int, map<int, vector<int>>>>>::iterator it = set.upper_bound(val);
	if (it == set.begin()) {
		return it;
	}
	else if ((--it)->first < val) {
		return ++it;
	}
	else
		return it;
}

map<int, map<int, map<int, vector<int>>>>::iterator \
greaterEqu2(map<int, map<int, map<int, vector<int>>>> &set, int val) {
	map<int, map<int, map<int, vector<int>>>>::iterator it = set.upper_bound(val);
	if (it == set.begin()) {
		return it;
	}
	else if ((--it)->first < val) {
		return ++it;
	}
	else
		return it;
}

map<int, map<int, vector<int>>>::iterator \
greaterEqu3(map<int, map<int, vector<int>>> &set, int val) {
	map<int, map<int, vector<int>>>::iterator it = set.upper_bound(val);
	if (it == set.begin()) {
		return it;
	}
	else if ((--it)->first < val) {
		return ++it;
	}
	else
		return it;
}

map<int, vector<int>>::iterator \
greaterEqu4(map<int, vector<int>> &set, int val) {
	map<int, vector<int>>::iterator it = set.upper_bound(val);
	if (it == set.begin()) {
		return it;
	}
	else if ((--it)->first < val) {
		return ++it;
	}
	else
		return it;
}