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
	/**
	 * @brief This is an enum class
	 */	
	enum ComponentType { CPU, GPU, SD, NIC };

	/**
	 * @var {std::unordered_map<std::string, ComponentType>} componentMap
	 * @brief Stores the components
	 */
	extern std::unordered_map<std::string, ComponentType> componentMap;
	
    /**
	 * @brief Retrieves the height of the terminal window.
	 * @function getTerminalHeight
	 * @returns int The height of the terminal window.
	 */
	int getTerminalHeight();

	/**
	 * @brief Converts a wide std::wstring to a string.
	 * @function wstringToString
	 * @param {std::widestring} wide_string The wide string to convert.
	 * @return {std::string} The converted string.
	 */
	std::string wstringToString(const std::wstring& wide_string);
}

