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
		* @brief the name of the process
		*/
		std::string name;

		/**
		* @brief the pids of the process
		*/
		std::vector<int> pids;
		
		/**
		* @brief 
		*/
		std::map<ULONGLONG, IoEventInfo> irpMap;

		/**
		* @brief 
		*/
		bool cpuEnabled = false;

		/**
		* @brief the pids of the process
		*/
		bool gpuEnabled = false;

		/**
		* @brief the pids of the process
		*/
		bool sdEnabled = false;

		/**
		* @brief the pids of the process
		*/
		bool nicEnabled = false;

		/**
		* @brief the total energy used by cpu for this process
		*/
		double cpuEnergy = 0.0;

		/**
		* @brief the average power used by cpu for this process
		*/
		double avgCpuEnergy = 0.0;

		/**
		* @brief the total energy used by gpu for this process
		*/
		double gpuEnergy = 0.0;

		/**
		* @brief the average power used by gpu for this process
		*/
		double avgGpuEnergy = 0.0;

		/**
		* @brief the total energy used by sd for this process
		*/
		double sdEnergy = 0.0;

		/**
		* @brief the average power used by sd for this process
		*/
		double avgSdEnergy = 0.0;

		/**
		* @brief the total energy used by nic for this process
		*/
		double nicEnergy = 0.0;

		/**
		* @brief the average power used by nic for this process
		*/
		double avgNicEnergy = 0.0;

	public:

		/**
		* @brief Builds a new process 
		*
		* @param appName the name of the process
		* @param pids the list of pids of the process 
		*/
		MonitoringData(const std::string& appName = "", const std::vector<int>& pids = {}) : name(appName), pids(pids) {}

		/**
		* @brief Gets the name of the process
		*
		* @return string the name of the process
		*/
		std::string getName() const;

		/**
		* @brief Gets the pids of the process
		*
		* @return vector of int list of the pids of the process
		*/
		std::vector<int> getPids() const;

		/**
		* @brief Enables the chosen component for the monitoring of this process
		*
		* @param component the name of the component to enable
		*/
		void enableComponent(const std::string& component);

		/**
		* @brief Disables the chosen component for the monitoring of this process
		*
		* @param component the name of the component to disable
		*/
		void disableComponent(const std::string& component);

		/**
		* @brief Returns if the CPU is enabled for the monitoring of this process
		*
		* @return bool 
		*/
		bool isCPUEnabled() const;

		/**
		* @brief Returns if the GPU is enabled for the monitoring of this process
		*
		* @return bool 
		*/
		bool isGPUEnabled() const;

		/**
		* @brief Returns if the SD is enabled for the monitoring of this process
		*
		* @return bool 
		*/
		bool isSDEnabled() const;

		/**
		* @brief Returns if the NIC is enabled for the monitoring of this process
		*
		* @return bool 
		*/
		bool isNICEnabled() const;

		/**
		* @brief Gets the energy used by the CPU for this process
		*
		* @return double the energy in Joules used by the CPU for this process
		*/
		double getCPUEnergy() const;

		/**
		* @brief Gets the average power used by the CPU for this process
		*
		* @return double the average power in Watt used by the CPU for this process
		*/
		double getAvgCPUEnergy() const;

		/**
		* @brief Gets the energy used by the GPU for this process
		*
		* @return double the energy in Joules used by the GPU for this process
		*/
		double getGPUEnergy() const;

		/**
		* @brief Gets the average power used by the GPU for this process
		*
		* @return double the average power in Watt used by the GPU for this process
		*/
		double getAvgGPUEnergy() const;

		/**
		* @brief Gets the energy used by the SD for this process
		*
		* @return double the energy in Joules used by the SD for this process
		*/
		double getSDEnergy() const;

		/**
		* @brief Gets the average power used by the SD for this process
		*
		* @return double the average power in Watt used by the SD for this process
		*/
		double getAvgSDEnergy() const;

		/**
		* @brief Gets the energy used by the NIC for this process
		*
		* @return double the energy in Joules used by the NIC for this process
		*/
		double getNICEnergy() const;

		/**
		* @brief Gets the average power used by the NIC for this process
		*
		* @return double the average power in Watt used by the NIC for this process
		*/
		double getAvgNICEnergy() const;

		/**
		* @brief Updates energy used by the CPU for this process by adding the current energy with the last enregy calculated
		*
		* @param energy the last energy calculated 
		*/
		void updateCPUEnergy(double energy);

		/**
		* @brief Updates energy used by the GPU for this process by adding the current energy with the last enregy calculated
		*
		* @param energy the last energy calculated 
		*/
		void updateGPUEnergy(double energy);

		/**
		* @brief Updates energy used by the SD for this process by adding the current energy with the last enregy calculated
		*
		* @param energy the last energy calculated 
		*/
		void updateSDEnergy(double energy);

		/**
		* @brief Updates energy used by the NIC for this process by adding the current energy with the last enregy calculated
		*
		* @param energy the last energy calculated 
		*/
		void updateNICEnergy(double energy);


		void addIrp(ULONGLONG irpAddress, const IoEventInfo& info);
		void updateIrp(ULONGLONG irpAddress, ULONG bytesTransferred);
		void removeIrp(ULONGLONG irpAddress);
};

