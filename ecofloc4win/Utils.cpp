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
}