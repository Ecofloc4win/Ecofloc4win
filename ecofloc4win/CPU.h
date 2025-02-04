#pragma once

#include <iostream>
#include <Windows.h>
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;

namespace CPU
{
	double getCapacitance();
	bool getAvgFreq(double& freq);
	bool getAvgVolt(double& volt);
	uint64_t fromFileTime(const FILETIME& ft);
	uint64_t getCPUTime();
	uint64_t getPidTime(DWORD pid);
	bool getCurrentPower(double& power);
};