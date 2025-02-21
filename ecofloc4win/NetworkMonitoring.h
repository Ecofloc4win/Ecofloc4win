#pragma once

#define NOMINMAX

#include <winsock2.h>        // Include Winsock2 before windows.h to avoid conflicts
#include <WS2tcpip.h>
#include <windows.h>         // Windows core headers
#include <iphlpapi.h>        // Network management functions
#include <tcpestats.h>       // TCP extended stats
#include <vector>            // For vector container
#include <mutex>             // For mutex support in multithreading
#include <atomic>

#include "MonitoringData.h"

#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"

using namespace ftxui;

/**
* @brief Namespace for network monitoring related classes and functions.
*/
namespace NetworkMonitoring
{
	/**
	* @brief Class to monitor network interfaces.
	*/
    class NICMonitor
    {
    private:
        std::atomic<bool>& newData;
        std::mutex& dataMutex;
        std::vector<MonitoringData>& monitoringData;
        ScreenInteractive& screen;
        int interval;
        std::vector<MonitoringData> localMonitoringData;

        struct TCPTableWrapper
        {
            std::unique_ptr<BYTE[]> buffer;
            PMIB_TCPTABLE_OWNER_PID tcpTable;

            /**
			* @brief Constructor for the TCPTableWrapper class.
            */
            bool initialize();
        };

        /**
		* @brief Gets the TCP table.
        * 
		* @param tcpTable The TCP table
        */
        void processConnection(const MIB_TCPROW_OWNER_PID& row);

        /**
		* @brief Processes the monitoring data for the network interfaces.
        * 
		* @param data The monitoring data
        */
        void updateEnergyMetrics(PTCP_ESTATS_DATA_ROD_v0 dataRod);

        /**
		* @brief Processes the monitoring data for the network interfaces.
        */
        void processMonitoringData();

    public:
        /**
		* @brief Constructor for the NICMonitor class.
        */
        NICMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs)
            : newData(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs)
        {
        }

        /**
		* @brief Runs the NIC monitoring process.
        */
        void run();
    };
}
