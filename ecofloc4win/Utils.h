/**
 * @file Utilis.h
 * @brief Implementation of useful function.
 * @author Ecofloc's Team
 * @date 2025-02-03
 */

#pragma once

#define WIN32_LEAN_AND_MEAN  // Prevent inclusion of unnecessary Windows headers

#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <windows.h>  // Ajout pour DWORD

/**
 * @namespace Utils
 * @brief Namespace for useful functionalities.
 */
namespace Utils
{
	enum ComponentType { CPU, GPU, SD, NIC };

	//std::unordered_map<std::string, ComponentType> componentMap =
	//{
	//	{ "CPU", ComponentType::CPU },
	//	{ "GPU", ComponentType::GPU },
	//	{ "SD", ComponentType::SD },
	//	{ "NIC", ComponentType::NIC }
	//};

	extern std::unordered_map<std::string, ComponentType> componentMap;
	
    /**
	 * @brief Retrieves the height of the terminal window.
	 *
	 * @return int The height of the terminal window.
	 */
	int getTerminalHeight();

	/**
	 * @brief Converts a wide std::wstring to a std::string.
	 *
	 * @param std::wide_string The wide std::string to convert.
	 * @return std::string The converted std::string.
	 */
	std::string wstringToString(const std::wstring& wide_string);

	/**
	 * @brief Update a specified JSON file for a pid with given metrics.
	 *
	 * @return std::string The converted std::string.
	 */
	std::string generateRandomColor();

	/**
	 * @brief Update a specified JSON file for a pid with given metrics.
	 *
	 * @param const std::string& filename the path of the file to update
	 * @param int pid the pid of the process to which metrics belong
	 * @param double valueCPU variable containing the energy value to be updated for the cpu
	 * @param double valueGPU variable containing the energy value to be updated for the gpu
	 * @param double valueNIC variable containing the energy value to be updated for the nic
	 * @param double valueSD variable containing the energy value to be updated for the sd
	 * @return std::string The converted std::string.
	 */
	void updateJsonFile(const std::string& filename, int pid, double valueCPU, double valueGPU, double valueNIC, double valueSD, std::mutex& mutex);

	/**
	 * @brief Retrieves the PIDs of processes with a given name
	 *
	 * @param processName The name of the process to search for
	 * @return std::vector<DWORD> List of found PIDs
	 */
	std::vector<DWORD> getPIDsByName(const std::string& processName);
}

