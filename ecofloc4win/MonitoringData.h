/**
 * @file MonitoringData.h
 * @brief Implementation of process-component management features.
 * @author Ecofloc's Team
 * @date 2025-02-03
 */

#pragma once

#include <vector>
#include <string>
#include <windows.h>
#include <map>

struct IoEventInfo {
	DWORD pid;
	std::wstring processName;
	USHORT operationType;
	ULONG bytesTransferred = 0;  // Track total bytes transferred for each IRP
};

/**
 * @class MonitoringData
 * @brief Class to manage process-component operations.
 *
 * This class provides functionalities to enable and disable any component to work on the process
 * and update the energy used by each active component for the process and also get the total used
 */
class MonitoringData
{
	private:

		/**
		* @var {std::string} name
		* @brief the name of the process
		*/
		std::string name;

		/**
		* @var {std::vector<int>} pids
		* @brief the pids of the process
		*/
		std::vector<int> pids;
		
		/**
		* @var {std::map<ULONGLONG, IoEventInfo>} irpMap
		* @brief 
		*/
		std::map<ULONGLONG, IoEventInfo> irpMap;

		/**
		* @var {bool} cpuEnabled
		* @brief true if cpu enable false otherwise
		*/
		bool cpuEnabled = false;

		/**
		* @var {bool} gpuEnabled
		* @brief true if gpu enable false otherwise
		*/
		bool gpuEnabled = false;

		/**
		* @var {bool} sdEnabled
		* @brief true if cd enable false otherwise
		*/
		bool sdEnabled = false;

		/**
		* @var {bool} nicEnabled
		* @brief true if nic enable false otherwise
		*/
		bool nicEnabled = false;

		/**
		* @var {double} cpuEnergy
		* @brief the total energy used by cpu for this process
		*/
		double cpuEnergy = 0.0;

		/**
		* @var {double} gpuEnergy
		* @brief the total energy used by gpu for this process
		*/
		double gpuEnergy = 0.0;

		/**
		* @var {double} sdEnergy
		* @brief the total energy used by sd for this process
		*/
		double sdEnergy = 0.0;

		/**
		* @var {double} nicEnergy
		* @brief the total energy used by nic for this process
		*/
		double nicEnergy = 0.0;

	public:

		/**
		* @brief Builds a new process 
		* 
		* @param {std::string} appName - the name of the process
		* @param {std::vector<int>} pids - the list of pids of the process 
		*/
		MonitoringData(const std::string& appName = "", const std::vector<int>& pids = {}) : name(appName), pids(pids) {}

		/**
		* @brief Gets the name of the process
		* @function getName
		* @returns {std::string} the name of the process
		*/
		std::string getName() const;

		/**
		* @brief Gets the pids of the process
		* @function getPids
		* @returns {std::vector<int>} of int list of the pids of the process
		*/
		std::vector<int> getPids() const;

		/**
		* @brief Enables the chosen component for the monitoring of this process
		* @function enableComponent
		* @param {std::string} component - the name of the component to enable
		*/
		void enableComponent(const std::string& component);

		/**
		* @brief Disables the chosen component for the monitoring of this process
		* @function disableComponent
		* @param {std::string} component - the name of the component to disable
		*/
		void disableComponent(const std::string& component);

		/**
		* @brief Returns if the CPU is enabled for the monitoring of this process
		* @function isCPUEnabled
		* @returns {bool} true if enabled false otherwise
		*/
		bool isCPUEnabled() const;

		/**
		* @brief Returns if the GPU is enabled for the monitoring of this process
		* @function isGPUEnabled
		* @returns {bool} true if enabled false otherwise
		*/
		bool isGPUEnabled() const;

		/**
		* @brief Returns if the SD is enabled for the monitoring of this process
		* @function isSDEnabled
		* @returns {bool} true if enabled false otherwise
		*/
		bool isSDEnabled() const;

		/**
		* @brief Returns if the NIC is enabled for the monitoring of this process
		* @function isNICEEnabled
		* @returns {bool} true if enabled false otherwise
		*/
		bool isNICEnabled() const;

		/**
		* @brief Gets the energy used by the CPU for this process
		* @function getCPUEnergy
		* @returns {double} the energy in Joules used by the CPU for this process
		*/
		double getCPUEnergy() const;

		/**
		* @brief Gets the energy used by the GPU for this process
		* @function getGPUEnergy
		* @returns {double} the energy in Joules used by the GPU for this process
		*/
		double getGPUEnergy() const;

		/**
		* @brief Gets the energy used by the SD for this process
		* @function getSDEnergy
		* @returns {double} the energy in Joules used by the SD for this process
		*/
		double getSDEnergy() const;

		/**
		* @brief Gets the energy used by the NIC for this process
		* @function getNICEnergy
		* @returns {double} the energy in Joules used by the NIC for this process
		*/
		double getNICEnergy() const;

		/**
		* @brief Updates energy used by the CPU for this process by adding the current energy with the last enregy calculated
		* @function updateCPUEnergy
		* @param {double} energy - the last energy calculated 
		*/
		void updateCPUEnergy(double energy);

		/**
		* @brief Updates energy used by the GPU for this process by adding the current energy with the last enregy calculated
		* @function updateGPUEnergy
		* @param {double} energy - the last energy calculated 
		*/
		void updateGPUEnergy(double energy);

		/**
		* @brief Updates energy used by the SD for this process by adding the current energy with the last enregy calculated
		* @function updateSDEnergy
		* @param {double} energy - the last energy calculated 
		*/
		void updateSDEnergy(double energy);

		/**
		* @brief Updates energy used by the NIC for this process by adding the current energy with the last enregy calculated
		* @function updateNICEnergy
		* @param {double} energy - the last energy calculated 
		*/
		void updateNICEnergy(double energy);


		void addIrp(ULONGLONG irpAddress, const IoEventInfo& info);
		void updateIrp(ULONGLONG irpAddress, ULONG bytesTransferred);
		void removeIrp(ULONGLONG irpAddress);
};

