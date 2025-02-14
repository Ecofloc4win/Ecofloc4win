/**
 * @file GPU.h
 * @brief Implementation of GPU management features.
 * @author Ecofloc's Team
 * @date 2025-02-03
 */

#pragma once

#include <vector>

/**
 * @namespace GPU
 * @brief Namespace for GPU-related functionalities.
 */
namespace GPU
{
	/**
	 * @brief Gets all usage of the GPU for each pid to monitor
	 *
	 * @param pids list of pids
	 * @return vector of int list of all usage for each pid
	 */
	std::vector<int> getGPUUsage(std::vector<int> pids);//

	/**
	 * @brief Gets the current power used by the GPU
	 *
	 * @return int the current power in Watt used by the GPU
	 */
	int getGPUPower();

	/**
	 * @brief Gets the total energy used by the GPU
	 *
	 * @param pids list of pids
	 * @param ms the interval in millisecond
	 * @return int the total energy in Joules used by the GPU
	 */
	int getGPUJoules(std::vector<int> pids, int ms);
};
