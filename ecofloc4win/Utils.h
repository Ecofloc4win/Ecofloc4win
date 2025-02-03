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

	std::unordered_map<std::string, ComponentType> componentMap =
	{
		{ "CPU", ComponentType::CPU },
		{ "GPU", ComponentType::GPU },
		{ "SD", ComponentType::SD },
		{ "NIC", ComponentType::NIC }
	};
	
    /**
	 * @brief Retrieves the height of the terminal window.
	 *
	 * @return int The height of the terminal window.
	 */
	int getTerminalHeight();

	/**
	 * @brief Converts a wide string to a string.
	 *
	 * @param wide_string The wide string to convert.
	 * @return string The converted string.
	 */
	string wstringToString(const wstring& wide_string);
}

