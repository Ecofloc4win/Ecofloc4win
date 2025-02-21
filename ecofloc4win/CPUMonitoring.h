#pragma once

#define NOMINMAX

#include <vector>
#include <string>
#include <windows.h>
#include <atomic>
#include <mutex>
#include "MonitoringData.h"
#include "CPU.h"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"

using namespace ftxui;

/**
* @brief Namespace for CPU monitoring related classes and functions.
*/
namespace CPUMonitoring {
	/**
	* @brief Class to monitor CPU devices.
	*/
    class CPUMonitor {
    private:
        std::atomic<bool>& newData;
        std::mutex& dataMutex;
        std::vector<MonitoringData>& monitoringData;
        ScreenInteractive& screen;
        int interval;
        std::vector<MonitoringData> localMonitoringData;

        struct PowerMeasurement {
            double startPower;
            double endPower;
            double average() const { return (startPower + endPower) / 2; }
        };

        struct TimeSnapshot {
            uint64_t cpuTime;
            uint64_t pidTime;
        };

        /**
		* @brief Updates the local monitoring data.
        */
        void updateLocalData();

		/**
		* @brief Measures the power consumption over an interval.
        * 
		* @return PowerMeasurement the power measurement
        */
        PowerMeasurement measurePowerOverInterval();

        /**
		* @brief Takes a snapshot of the CPU and process time.
        * 
		* @param pid The process ID
		* @return TimeSnapshot the time snapshot
        */
        TimeSnapshot takeTimeSnapshot(const int pid);

        /**
		* @brief Validates the time difference between two snapshots.
        * 
		* @param start The start snapshot
		* @param end The end snapshot
		* @return bool true if the time difference is valid, false otherwise
        */
        bool validateTimeDifference(const TimeSnapshot& start, const TimeSnapshot& end);

        /**
		* @brief Calculates the CPU usage over an interval.
        * 
		* @param start The start snapshot
		* @param end The end snapshot
		* @return double the CPU usage
        */
        double calculateCPUUsage(const TimeSnapshot& start, const TimeSnapshot& end);

        /**
		* @brief Calculates the energy consumption over an interval.
        * 
		* @param powerAvg The average power consumption
		* @param cpuUsage The CPU usage
		* @return double the energy consumption
        */
        double calculateIntervalEnergy(double powerAvg, double cpuUsage);

        /**
		* @brief Updates the monitoring data with the energy consumption.
        * 
		* @param pids The process IDs
		* @param energy The energy consumption
		* @return bool true if the monitoring data is valid, false otherwise
        */
        void updateMonitoringDataEnergy(const std::vector<int>& pids, double energy);

        /**
		* @brief Checks if the monitoring data is valid.
        * 
		* @param data The monitoring data
		* @return bool true if the monitoring data is valid, false otherwise
        */
        bool isValidMonitoringData(const MonitoringData& data);

        /**
		* @brief Processes the monitoring data for the CPU devices.
        */
        void processMonitoringData();

    public:
        /**
		* @brief Constructor for the CPUMonitor class.
        */
        CPUMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs)
            : newData(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs)
        {
        }

        /**
		* @brief Destructor for the CPUMonitor class.
        */
        ~CPUMonitor() = default;

        /**
		* @brief Runs the CPU monitoring process.
        */
        void run();
    };
}