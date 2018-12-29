#pragma once
#include <fstream>
#define SAFE_RELEASE(x) if(x) { x->Release(); x = 0;}

inline bool fileExists(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}