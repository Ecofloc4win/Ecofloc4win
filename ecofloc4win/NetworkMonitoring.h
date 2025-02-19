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

namespace NetworkMonitoring
{
    class NICMonitor
    {
    private:
        std::atomic<bool>& newData;
        std::mutex& dataMutex;
        std::vector<MonitoringData>& monitoringData;
        ScreenInteractive& screen;
        int interval;
        int timeout;
        std::vector<MonitoringData> localMonitoringData;

        struct TCPTableWrapper
        {
            std::unique_ptr<BYTE[]> buffer;
            PMIB_TCPTABLE_OWNER_PID tcpTable;

            bool initialize();
        };

        void processConnection(const MIB_TCPROW_OWNER_PID& row);

        void updateEnergyMetrics(PTCP_ESTATS_DATA_ROD_v0 dataRod);

        void processMonitoringData();

    public:
        NICMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs)
            : newData(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs), timeout(-1)
        {
        }

        NICMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs, int timeout)
            : newData(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs), timeout(timeout)
        {
        }

        void run();
    };
}
