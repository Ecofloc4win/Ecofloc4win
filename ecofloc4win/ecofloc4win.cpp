/**
 * @file ecofloc4win.cpp
 * @brief The main program.
 * @author Ecofloc's Team
 * @date 2025-01-06
 */

#define NOMINMAX
//#define WIN32_LEAN_AND_MEAN  // Prevent inclusion of unnecessary Windows headers
#define WIN32_WINNT 0x0600  // Vista and later

#include <winsock2.h>        // Include Winsock2 before windows.h to avoid conflicts
#include <WS2tcpip.h>
#include <windows.h>         // Windows core headers
#include <iphlpapi.h>        // Network management functions
#include <tcpestats.h>       // TCP extended stats
#include <psapi.h>           // For ProcessStatus API
#include <tchar.h>           // Generic text mappings for Unicode/ANSI
#include <locale>            // For localization and locale functions
#include <codecvt>           // For string conversions
#include <tlhelp32.h>        // For process and snapshot handling
#include <cctype>            // For character handling functions
#include <algorithm>         // For STL algorithms
#include <unordered_map>     // For unordered map functionality
#include <utility>           // For utility functions and data types
#include <vector>            // For vector container
#include <string>            // For string handling
#include <sstream>           // For string streams
#include <list>              // For list container
#include <mutex>             // For mutex support in multithreading
#include <Pdh.h>
#include <PdhMsg.h>
#include <cstring>
#include <iostream>
#include <tcpmib.h>
#include <atomic>
#include <unordered_set>

#include "process.h"         // Custom header for process handling
#include "GPU.h"             // Custom header for GPU monitoring
#include "CPU.h"
#include "MonitoringData.h"  // Custom header for monitoring data
#include "Utils.h"
#include "NetworkMonitoring.h"
#include "StorageMonitoring.h"
#include "GPUMonitoring.h"

#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/table.hpp"
#include "ftxui/dom/node.hpp"
#include "ftxui/screen/color.hpp"
#include "CPUMonitoring.h"

using namespace ftxui;

/**
 * @brief The array that content all the energy used by each component for each process
 */
std::vector<MonitoringData> monitoringData = {};

/**
 * @brief Protects shared data
 */
std::mutex dataMutex;

/**
 * @brief Stores CPU newest data
 */
std::atomic<bool> newDataCpu(false);

/**
 * @brief Stores GPU newest data
 */
std::atomic<bool> newDataGpu(false);

/**
 * @brief Stores SD newest data
 */
std::atomic<bool> newDataSd(false);

/**
 * @brief Stores NIC newest data
 */
std::atomic<bool> newDataNic(false);

/**
 * @brief List of action you can do when using ecofloc
 */
std::unordered_map<std::string, int> actions =
{
	{"enable", 1},
	{"disable", 2},
	{"add", 3},
	{"remove", 4},
	{"interval", 5},
	{"quit", 6},
};

/**
 * 
 *
 * can be replace
 */
std::unordered_map<std::string, std::pair<std::vector<process>, bool>> comp = { {"CPU", {{}, false}}, {"GPU", {{}, false} }, {"SD", {{}, false }}, {"NIC", {{}, false }} };

/**
 * @brief The time interval between 2 calculations in millisecond
 */
std::atomic<int> interval = 500;

/**
 * @brief Reads the command written by the user and called the right function
 * 
 * @param commandHandle the command written by the user
 */
void readCommand(std::string commandHandle);

/**
 * @brief Adds a process to monitor on the chosen component
 *
 * @param pid The ID of the process to add
 * @param component The name of the component where the pid is added
 */
void addProcPid(const std::string& pid, const std::string& component);

/**
 * @brief Adds a process to monitor on the chosen component
 *
 * @param name The name of the process to add
 * @param component The name of the component where the pid is added
 */
void addProcName(const std::string& name, const std::string& component);

/**
 * @brief Removes a process from monitoring
 *
 * @param lineNumber The number of the line that match the process to remove
 */
void removeProcByLineNumber(const std::string& lineNumber) noexcept;

/**
 * @brief Enables the monitoring of the component for a specified process
 *
 * @param lineNumber The number of the line that match the process wanted
 * @param component The name of the component to enable
 */
void enable(const std::string& lineNumber, const std::string& component);

/**
 * @brief Disables the monitoring of the component for a specified process
 *
 * @param lineNumber The number of the line that match the process wanted
 * @param component The name of the component to disable
 */
void disable(const std::string& lineNumber, const std::string& component);

/**
 * @brief Gets the localized counter path for a given process name and counter name to be used in PDH functions
 * 		  and avoid hardcoding the counter path for each language.
 *
 * @param processName The name of the process
 * @param counterName The name of the counter
 * @return wstring the localized counter path for the process
 */
std::wstring getLocalizedCounterPath(const std::wstring& processName, const std::string& counterName);

/**
 * @brief Retrieves the name of the Process thank to its ID
 *
 * @param processID The ID of the Process
 * @return wstring the name of the Process
 */
std::wstring getProcessNameByPID(DWORD processID);

/**
 * @brief Gets the index of a counter in the registry based on its name.
 *
 * @param counterName The name of the counter
 * @return DWORD the Index of the counter
 */
DWORD getCounterIndex(const std::string& counterName);

/**
 * @brief Gets the name of the instance for a given process ID.
 * 
 * @param targetPID The pid
 * @return wstring the name of the instance
 */
std::wstring getInstanceForPID(int targetPID);

/**
 * @brief Generates all rows of the table to show in the terminal
 * 
 * @return vector of vectors of strings the results' table 
 */
auto createTableRows() -> std::vector<std::vector<std::string>>
{
	std::vector<std::vector<std::string>> rows;
	std::lock_guard<std::mutex> lock(dataMutex);

	int rowNumber = 1;

	rows.emplace_back(std::vector<std::string>{"Line", "Application Name", "CPU", "GPU", "SD", "NIC"});
	for (const auto& data : monitoringData)
	{
		std::ostringstream cpuEnergyStream;
		cpuEnergyStream << std::fixed << std::setprecision(2) << data.getCPUEnergy();

		std::ostringstream gpuEnergyStream;
		gpuEnergyStream << std::fixed << std::setprecision(2) << data.getGPUEnergy();

		std::ostringstream sdEnergyStream;
		sdEnergyStream << std::fixed << std::setprecision(2) << data.getSDEnergy();

		std::ostringstream nicEnergyStream;
		nicEnergyStream << std::fixed << std::setprecision(2) << data.getNICEnergy();

		rows.emplace_back(std::vector<std::string>
		{
			std::to_string(rowNumber),
			data.getName(),
			" " + cpuEnergyStream.str() + " J ",
			" " + gpuEnergyStream.str() + " J ",
			" " + sdEnergyStream.str() + " J ",
			" " + nicEnergyStream.str() + " J "
		});
		rowNumber++;
	}

	return rows;
}

/**
 * @brief Shows the table in the terminal
 *
 * @param scrollPosition The current position of the scroll
 * @return Element the render of the complete table 
 */
auto renderTable(int scrollPosition) -> Element
{
	auto tableData = createTableRows();
	int terminalHeight = Utils::getTerminalHeight();
	int visibleRows = terminalHeight - 8; // Adjust for input box and borders

	// Prepare rows for the visible portion
	std::vector<std::vector<std::string>> visibleTableData;
	visibleTableData.push_back(tableData[0]); // Header row
	for (int i = scrollPosition + 1; i < std::min(scrollPosition + visibleRows + 1, (int)tableData.size()); ++i)
	{
		visibleTableData.push_back(tableData[i]);
	}

	auto table = Table(visibleTableData);

	// Style the table
	table.SelectAll().Border(LIGHT);
	table.SelectRow(0).Decorate(bold);
	table.SelectRow(0).DecorateCells(center);
	table.SelectRow(0).SeparatorVertical(LIGHT);
	table.SelectRow(0).Border();
	table.SelectColumn(0).DecorateCells(center);
	table.SelectColumn(1).Decorate(flex);
	table.SelectColumns(0, -1).SeparatorVertical(LIGHT);
	auto content = table.SelectRows(1, -1);
	content.DecorateCellsAlternateRow(color(Color::Red), 3, 0);
	content.DecorateCellsAlternateRow(color(Color::RedLight), 3, 1);
	content.DecorateCellsAlternateRow(color(Color::White), 3, 2);

	return table.Render() | flex;
}

int main()
{
	std::string input;
	Component inputBox = Input(&input, "Type here");
	inputBox |= CatchEvent([&](Event event)
	{
		if (event == Event::Return)
		{
			if (!input.empty())
			{
				readCommand(input);
				input.clear();
			}
			return true;
		}
		return false;
	});

	auto screen = ScreenInteractive::Fullscreen();

	// State variables for scrolling
	int scrollPosition = 0;

	// Component to handle input and update the scroll position
	auto component = Renderer(inputBox, [&]
	{
		return vbox(
			{
				renderTable(scrollPosition),
				separator(),
				hbox(
				{
					text("Command: "), inputBox->Render()
				}),
			}) | border;
	});

	component = CatchEvent(component, [&](Event event)
	{
		int terminalHeight = Utils::getTerminalHeight();
		int visibleRows = terminalHeight - 8;

		if ((int)monitoringData.size() <= visibleRows)
		{
			scrollPosition = 0; // Disable scrolling if all rows fit
			return false;
		}

		// Handle mouse wheel and arrow key events
		if (event.is_mouse())
		{
			if (event.mouse().button == Mouse::WheelDown)
			{
				scrollPosition = std::min(scrollPosition + 1, (int)monitoringData.size() - visibleRows - 1);
				return true;
			}

			if (event.mouse().button == Mouse::WheelUp)
			{
				scrollPosition = std::max(scrollPosition - 1, 0);
				return true;
			}
		}

		if (event == Event::ArrowDown)
		{
			scrollPosition = std::min(scrollPosition + 1, (int)monitoringData.size() - visibleRows - 1);
			return true;
		}

		if (event == Event::ArrowUp)
		{
			scrollPosition = std::max(scrollPosition - 1, 0);
			return true;
		}

		return false;
	});

	GPUMonitoring::GPUMonitor gpuMonitor(newDataGpu, dataMutex, monitoringData, screen, interval);
	std::thread gpuThread([&gpuMonitor] { gpuMonitor.run(); });

	StorageMonitoring::SDMonitor sdMonitor(newDataSd, dataMutex, monitoringData, screen, interval);
	std::thread sdThread([&sdMonitor] { sdMonitor.run(); });

	NetworkMonitoring::NICMonitor nicMonitor(newDataNic, dataMutex, monitoringData, screen, interval);
	std::thread nicThread([&nicMonitor] { nicMonitor.run(); });

	CPUMonitoring::CPUMonitor cpuMonitor(newDataCpu, dataMutex, monitoringData, screen, interval);
	std::thread cpuThread([&cpuMonitor] { cpuMonitor.run(); });

	// Run the application
	screen.Loop(component);

	gpuThread.join();
	sdThread.join();
	nicThread.join();
	cpuThread.join();
	return 0;
}

std::wstring getProcessNameByPID(DWORD processID)
{
	TCHAR processName[MAX_PATH] = TEXT("<unknown>");

	// Get a handle to the process
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

	// Check if we successfully got a handle to the process
	if (hProcess) {
		// Get the process name
		if (GetModuleBaseName(hProcess, nullptr, processName, sizeof(processName) / sizeof(TCHAR))) {
			CloseHandle(hProcess); // Close handle when done
			return processName;
		}
		CloseHandle(hProcess); // Close handle even if we fail
	}

	return L"<unknown>";
}

void readCommand(std::string commandHandle)
{
	std::istringstream tokenStream(commandHandle);

	std::vector<std::string> chain;

	while (getline(tokenStream, commandHandle, ' '))
	{
		chain.push_back(commandHandle);
	}

	switch (actions[chain[0]])
	{
	case 1:
		if (chain.size() == 3)
		{
			enable(chain[1], chain[2]);
		}
		else
		{
			std::cout << "error, need 2 in total for enable and disable" << std::endl;
		}
		break;

	case 2:
		if (chain.size() == 3)
		{
			disable(chain[1], chain[2]);
		}
		else
		{
			std::cout << "error, need 2 in total for enable and disable" << std::endl;
		}
		break;

	case 3:
		if (chain.size() == 4)
		{
			if (chain[1] == "-p")
			{
				if (all_of(chain[2].begin(), chain[2].end(), ::isdigit))
				{
					if (chain[3] == "CPU" || chain[3] == "GPU" || chain[3] == "SD" || chain[3] == "NIC")
					{
						addProcPid(chain[2], chain[3]);
					}
					else
					{
						std::cout << "error fourth argument (must be CPU, GPU, SD or NIC)" << std::endl;
					}
				}
				else
				{
					std::cout << "error third argument (must be an integer)" << std::endl;
				}
			}
			else if (chain[1] == "-n")
			{
				if (chain[3] == "CPU" || chain[3] == "GPU" || chain[3] == "SD" || chain[3] == "NIC")
				{
					addProcName(chain[2], chain[3]);
				}
				else
				{
					std::cout << "error fourth argument (must be CPU, GPU, SD or NIC)" << std::endl;
				}
			}
			else
			{
				std::cout << "error second argument (-p for pid / -n for name)" << std::endl;
			}
		}
		else
		{
			std::cout << "error, need 4 in total for add and remove" << std::endl;
		}
		break;

	case 4:
		if (chain.size() == 2)
		{
			if (all_of(chain[1].begin(), chain[1].end(), ::isdigit))
			{
				removeProcByLineNumber(chain[1]);
			}
			else
			{
				std::cout << "error third argument (must be an integer)" << std::endl;
			}
		}
		else
		{
			std::cout << "error, need 4 in total for add and remove" << std::endl;
		}
		break;

	case 5:
		if (chain.size() == 2)
		{
			if (all_of(chain[1].begin(), chain[1].end(), ::isdigit))
			{
				interval = stoi(chain[1]);
				std::cout << "Interval has been changed" << std::endl;
			}
			else
			{
				std::cout << "error second argument (must be an integer)" << std::endl;
			}
		}
		else
		{
			std::cout << "error, need 2 in total for interval" << std::endl;
		}
		break;

	case 6:
		break;

	default:
		std::cout << "error first argument (list command: add/remove/enable/disable/interval/start/quit)" << std::endl;
		break;
	}
}

void addProcPid(const std::string& pid, const std::string& component)
{
	try
	{
		int processId = std::stoi(pid); // Convert PID to integer
		std::wstring processName = getProcessNameByPID(processId);

		// Check if the process name is valid
		if (processName.empty())
		{
			std::cerr << "Error: Invalid PID or process not found." << std::endl;
			return;
		}

		{
			std::unique_lock<std::mutex> lock(dataMutex);

			// Check for duplicate before adding
			auto it = std::find_if(monitoringData.begin(), monitoringData.end(),
				[processId](const MonitoringData& data)
			{
				return std::find(data.getPids().begin(), data.getPids().end(), processId) != data.getPids().end();
			});

			if (it == monitoringData.end())
			{
				MonitoringData data(Utils::wstringToString(processName), { processId });
				data.enableComponent(component);
				monitoringData.push_back(data);
				auto it2 = Utils::componentMap.find(component);
				if (it2 != Utils::componentMap.end())
				{
					it2->second;
				}
				
				switch (it2->second)
				{
				case Utils::CPU:
					newDataCpu.store(true, std::memory_order_release);
					break;
				case Utils::GPU:
					newDataGpu.store(true, std::memory_order_release);
					break;
				case Utils::SD:
					newDataSd.store(true, std::memory_order_release);
					break;
				case Utils::NIC:
					newDataNic.store(true, std::memory_order_release);
					break;
				}
			}
			else
			{
				std::cerr << "Warning: Process with PID " << pid << " is already being monitored." << std::endl;
			}
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Error: Exception while adding PID " << pid << ": " << ex.what() << std::endl;
	}
}


void addProcName(const std::string& name, const std::string& component)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Error: Unable to create process snapshot." << std::endl;
		return;
	}

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	std::wstring wstr(name.begin(), name.end());
	std::vector<int> pids;

	if (Process32First(hSnapshot, &pe32))
	{
		do
		{
			if (_wcsicmp(pe32.szExeFile, wstr.c_str()) == 0) // Case-insensitive comparison
			{
				int processId = pe32.th32ProcessID;

				// Check for duplicates in `comp[component].first`
				auto it = std::find_if(comp[component].first.begin(), comp[component].first.end(),
					[&processId](const process& o)
				{
					return o.getPid() == std::to_string(processId);
				});

				if (it == comp[component].first.end())
				{
					pids.push_back(processId);
					comp[component].first.push_back(process(std::to_string(processId), name));
				}
			}
		} while (Process32Next(hSnapshot, &pe32));
	}

	CloseHandle(hSnapshot);

	// Add valid processes to monitoringData
	if (!pids.empty())
	{
		{
			std::unique_lock<std::mutex> lock(dataMutex);
			MonitoringData data(name, pids);
			data.enableComponent(component);
			monitoringData.push_back(data);
			auto it2 = Utils::componentMap.find(component);
			if (it2 != Utils::componentMap.end())
			{
				it2->second;
			}

			switch (it2->second)
			{
			case Utils::CPU:
				newDataCpu.store(true, std::memory_order_release);
				break;
			case Utils::GPU:
				newDataGpu.store(true, std::memory_order_release);
				break;
			case Utils::SD:
				newDataSd.store(true, std::memory_order_release);
				break;
			case Utils::NIC:
				newDataNic.store(true, std::memory_order_release);
				break;
			}
		}
	}
	else
	{
		std::cerr << "Error: No processes found with name " << name << "." << std::endl;
	}
}


void removeProcByLineNumber(const std::string& lineNumber) noexcept
{
	try
	{
		int line = std::stoi(lineNumber);
		if (line < 0)
		{
			std::cerr << "Error: Line number cannot be negative." << std::endl;
			return;
		}

		std::unique_lock<std::mutex> lock(dataMutex);
		if (line >= monitoringData.size())
		{
			std::cerr << "Error: Line number is out of range." << std::endl;
			return;
		}

		// Store PIDs to remove before modifying containers
		const auto& data = monitoringData[line];
		std::unordered_set<unsigned int> pidsToRemove;
		pidsToRemove.reserve(data.getPids().size());  // Pre-allocate space

		for (const auto& pid : data.getPids())
		{
			pidsToRemove.insert(pid);
		}

		// Track components that will be affected for more detailed logging
		std::vector<std::string> affectedComponents;

		// More efficient removal from comp
		for (auto& [key, value] : comp)
		{
			auto& processes = value.first;
			auto originalSize = processes.size();

			// Use erase-remove idiom with an unordered_set for O(1) lookup
			processes.erase(
				std::remove_if(processes.begin(), processes.end(),
					[&pidsToRemove](const process& p)
				{
					return pidsToRemove.count(std::stoi(p.getPid())) > 0;
				}),
				processes.end()
			);

			// If processes were removed from this component, log it
			if (processes.size() < originalSize)
			{
				affectedComponents.push_back(key);
			}
		}

		// Remove from monitoringData
		monitoringData.erase(monitoringData.begin() + line);

		// Optional: Reduce memory footprint
		monitoringData.shrink_to_fit();

		// Atomic updates with memory ordering
		newDataCpu.store(true, std::memory_order_release);
		newDataGpu.store(true, std::memory_order_release);
		newDataSd.store(true, std::memory_order_release);
		newDataNic.store(true, std::memory_order_release);

		// Enhanced logging
		//std::cout << "Process has been removed from line " << line
		//	<< ". Affected components: ";
		for (const auto& component : affectedComponents)
		{
			//std::cout << component << " ";
		}
		//std::cout << std::endl;
	}
	catch (const std::invalid_argument& e)
	{
		std::cerr << "Error: Invalid line number. Must be a number." << std::endl;
	}
	catch (const std::out_of_range& e)
	{
		std::cerr << "Error: Line number is too large." << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unexpected error during process removal." << std::endl;
	}
}

void enable(const std::string& lineNumber, const std::string& component)
{
	try
	{
		int line = std::stoi(lineNumber);
		if (line < 0)
		{
			std::cerr << "Error: Line number cannot be negative." << std::endl;
			return;
		}

		std::unique_lock<std::mutex> lock(dataMutex);
		if (line >= monitoringData.size())
		{
			std::cerr << "Error: Line number is out of range." << std::endl;
			return;
		}

		auto& data = monitoringData[line];
		data.enableComponent(component);

		auto it = Utils::componentMap.find(component);
		if (it != Utils::componentMap.end())
		{
			it->second;
		}

		switch (it->second)
		{
		case Utils::CPU:
			newDataCpu.store(true, std::memory_order_release);
			break;
		case Utils::GPU:
			newDataGpu.store(true, std::memory_order_release);
			break;
		case Utils::SD:
			newDataSd.store(true, std::memory_order_release);
			break;
		case Utils::NIC:
			newDataNic.store(true, std::memory_order_release);
			break;
		}

		//std::cout << "Component " << component << " has been enabled for process " << data.getName() << " at line " << line << std::endl;

	}
	catch (const std::invalid_argument& e)
	{
		std::cerr << "Error: Invalid line number. Must be a number." << std::endl;
	}
	catch (const std::out_of_range& e)
	{
		std::cerr << "Error: Line number is too large." << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unexpected error during process removal." << std::endl;
	}
}

void disable(const std::string& lineNumber, const std::string& component)
{
	try
	{
		int line = std::stoi(lineNumber);
		if (line < 0)
		{
			std::cerr << "Error: Line number cannot be negative." << std::endl;
			return;
		}

		std::unique_lock<std::mutex> lock(dataMutex);
		if (line >= monitoringData.size())
		{
			std::cerr << "Error: Line number is out of range." << std::endl;
			return;
		}

		auto& data = monitoringData[line];
		data.disableComponent(component);

		auto it = Utils::componentMap.find(component);
		if (it != Utils::componentMap.end())
		{
			it->second;
		}

		switch (it->second)
		{
		case Utils::CPU:
			newDataCpu.store(true, std::memory_order_release);
			break;
		case Utils::GPU:
			newDataGpu.store(true, std::memory_order_release);
			break;
		case Utils::SD:
			newDataSd.store(true, std::memory_order_release);
			break;
		case Utils::NIC:
			newDataNic.store(true, std::memory_order_release);
			break;
		}

		//std::cout << "Component " << component << " has been enabled for process " << data.getName() << " at line " << line << std::endl;

	}
	catch (const std::invalid_argument& e)
	{
		std::cerr << "Error: Invalid line number. Must be a number." << std::endl;
	}
	catch (const std::out_of_range& e)
	{
		std::cerr << "Error: Line number is too large." << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unexpected error during process removal." << std::endl;
	}
}