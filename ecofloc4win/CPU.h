/**
 * @file CPU.h
 * @brief Implementation of CPU management features.
 * @author Ecofloc's Team
 * @date 2025-02-03
 */

#pragma once

#include <iostream>
#include <Windows.h>
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;

/**
 * @namespace CPU
 * @brief Namespace for CPU-related functionalities.
 */
namespace CPU
{	
	/**
	* @brief Retrieves the capacitance of the CPU.
    * 
	* @return double The capacitance of the CPU.
    */
    double getCapacitance();

    /**
	* @brief Retrieves the average frequency of the CPU.
    * 
	* @param freq A reference to a double where the average frequency will be stored.
	* @return bool True if the frequency was successfully retrieved, false otherwise.
    */
	bool getAvgFreq(double& freq);

    /**
	* @brief Retrieves the average voltage of the CPU.
    * 
	* @param volt A reference to a double where the average voltage will be stored.
	* @return bool True if the voltage was successfully retrieved, false otherwise.
    */
	bool getAvgVolt(double& volt);

	/**
     * @brief Converts a FILETIME structure to a uint64_t.
     *
     * @param ft The FILETIME structure to convert.
     * @return uint64_t The converted value.
     */
	uint64_t fromFileTime(const FILETIME& ft);

	/**
     * @brief Retrieves the total CPU time including idle, kernel, and user times.
     *
     * @return uint64_t The total CPU time in 100-nanosecond intervals. Returns -1 on failure.
     */
	uint64_t getCPUTime();

	/**
     * @brief Retrieves the total time spent by a process.
     *
     * @param pid The process ID of the target process.
     * @return uint64_t The total process time (kernel + user) in 100-nanosecond intervals. Returns -1 on failure.
     */
	uint64_t getPidTime(DWORD pid);

	/**
     * @brief Retrieves the current power consumption of the CPU.
     *
     * This function uses a DLL (`Wrapper.dll`) to retrieve the power consumption of CPU cores.
     *
     * @param power A reference to a double where the calculated power will be stored.
     * @return bool True if the power was successfully retrieved, false otherwise.
     */
	bool getCurrentPower(double& power);
};