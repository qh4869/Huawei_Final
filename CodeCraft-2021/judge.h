#pragma once
#include <iostream>
#include <utility>
#include <fstream>
#include <map>
#include <istream>
#include <string>
#include <vector>


struct Ser_t {
	int core, cpu, tot, day;
};

struct Ser {
	std::string name;
	std::pair<int, int> A, B;
	Ser() = default;
	Ser(std::string _name, const Ser_t& _s) {
		name = std::move(_name);
		A = { _s.core / 2, _s.cpu / 2 };
		B = { _s.core / 2, _s.cpu / 2 };
	}
};

struct Vir_t {
	int core, cpu, type;
};

struct Vir {
	std::string name;
	int core{}, cpu{}, type{};
	int pos{}, numa{};
	Vir() = default;
	Vir(std::string _name, const Vir_t& _v, int _pos, int _numa) {
		name = std::move(_name);
		core = _v.core;
		cpu = _v.cpu;
		type = _v.type;
		pos = _pos;
		numa = _numa;
	}
};


void runJudger(std::string inputFile, std::string outputFile);