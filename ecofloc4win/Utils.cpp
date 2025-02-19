/**
 * @file Utilis.cpp
 * @brief Definition of useful function.
 * @author Ecofloc's Team
 * @date 2025-02-03
 */

#define WIN32_LEAN_AND_MEAN  // Prevent inclusion of unnecessary Windows headers

#include "Utils.h"
#include <Windows.h>
#include <iostream>
#include "json.hpp"
#include <mutex>
#include <fstream>
#include <random>
#include <TlHelp32.h>
#include <algorithm>

using json = nlohmann::json;

/**
 * @namespace Utils
 * @brief Namespace for useful functionalities.
 */
namespace Utils
{
	std::unordered_map<std::string, ComponentType> componentMap =
	{
		{ "CPU", ComponentType::CPU },
		{ "GPU", ComponentType::GPU },
		{ "SD", ComponentType::SD },
		{ "NIC", ComponentType::NIC }
	};

	// Function to get terminal size
	int getTerminalHeight()
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
		{
			// Calculate the height of the terminal window.
			return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		}
		// Default to 24 rows if size cannot be determined.
		return 24;
	}

	std::string wstringToString(const std::wstring& wide_string)
	{
		std::string str;
		size_t size;
		str.resize(wide_string.length());
		wcstombs_s(&size, &str[0], str.size() + 1, wide_string.c_str(), wide_string.size());
		return str;
	}

    std::string generateRandomColor() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 255);
        char color[8];
        snprintf(color, sizeof(color), "#%02X%02X%02X", dist(gen), dist(gen), dist(gen));
        return std::string(color);
    }


    void updateJsonFile(const std::string& filename, int pid, double valueCPU, double valueGPU, double valueNIC, double valueSD, std::mutex& mutex) 
    {
        std::lock_guard<std::mutex> lock(mutex);

        json data;
        std::ifstream inFile(filename);
        if (inFile) 
        {
            inFile >> data;
            inFile.close();
        }
        else 
        {
            data = json{ {"apps", json::array()}, {"time", time(nullptr)} };
        }

        bool pidExists = false;
        for (auto& app : data["apps"]) 
        {
            if (app["pid"] == pid) 
            {
                pidExists = true;
                if (valueCPU > 0) app["power_w_CPU"] = valueCPU;
                if (valueGPU > 0) app["power_w_GPU"] = valueGPU;
                if (valueNIC > 0) app["power_w_NIC"] = valueNIC;
                if (valueSD > 0) app["power_w_SD"] = valueSD;
                break;
            }
        }

        if (!pidExists) {
            json newApp = {
                {"color", generateRandomColor()},
                {"pid", pid},
                {"power_w_CPU", valueCPU},
                {"power_w_GPU", valueGPU},
                {"power_w_NIC", valueNIC},
                {"power_w_SD", valueSD}
            };
            data["apps"].push_back(newApp);
        }

        data["time"] = time(nullptr);

        std::ofstream outFile(filename);
        outFile << data.dump(4);
        outFile.close();
    }

    std::vector<DWORD> getPIDsByName(const std::string& processName) {
        std::vector<DWORD> pids;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32W processEntry;
            processEntry.dwSize = sizeof(processEntry);
            
            std::string searchName = processName;
            std::transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);
            
            if (Process32FirstW(snapshot, &processEntry)) {
                do {
                    std::wstring wProcessName = processEntry.szExeFile;
                    std::string currentProcessName = wstringToString(wProcessName);
                    std::transform(currentProcessName.begin(), currentProcessName.end(), 
                                 currentProcessName.begin(), ::tolower);
                    
                    if (currentProcessName == searchName) {
                        pids.push_back(processEntry.th32ProcessID);
                    }
                } while (Process32NextW(snapshot, &processEntry));
            }
            CloseHandle(snapshot);
        }
        return pids;
    }
}