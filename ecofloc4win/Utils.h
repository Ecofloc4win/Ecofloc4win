/**
 * @file Utilis.h
 * @brief Implementation of useful function.
 * @author Ecofloc's Team
 * @date 2025-02-03
 */

#pragma once

#include <string>
#include <unordered_map>

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
}

