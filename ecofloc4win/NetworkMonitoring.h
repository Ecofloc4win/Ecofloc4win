#pragma once

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
#include "ftxui/screen/color.hpp"

using namespace ftxui;

namespace NetworkMonitoring
{
    class NICMonitor
    {
    private:
        std::atomic<bool>& newDataNic;
        std::mutex& dataMutex;
        std::vector<MonitoringData>& monitoringData;
        ScreenInteractive& screen;
        int interval;
        std::vector<MonitoringData> localMonitoringData;

        struct TCPTableWrapper
        {
            std::unique_ptr<BYTE[]> buffer;
            PMIB_TCPTABLE_OWNER_PID tcpTable;

            bool initialize()
            {
                ULONG ulSize = 0;
                if (GetExtendedTcpTable(nullptr, &ulSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) != ERROR_INSUFFICIENT_BUFFER) {
                    return false;
                }
                buffer = std::make_unique<BYTE[]>(ulSize);
                tcpTable = reinterpret_cast<PMIB_TCPTABLE_OWNER_PID>(buffer.get());
                return GetExtendedTcpTable(tcpTable, &ulSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR;
            }
        };
        void processConnection(const MIB_TCPROW_OWNER_PID& row)
        {
            if (row.dwState != MIB_TCP_STATE_ESTAB || row.dwRemoteAddr == htonl(INADDR_LOOPBACK))
            {
                return;
            }

            // Create a non-const copy for the reinterpret_cast
            MIB_TCPROW tcpRow;
            tcpRow.dwState = row.dwState;
            tcpRow.dwLocalAddr = row.dwLocalAddr;
            tcpRow.dwLocalPort = row.dwLocalPort;
            tcpRow.dwRemoteAddr = row.dwRemoteAddr;
            tcpRow.dwRemotePort = row.dwRemotePort;

            TCP_ESTATS_DATA_RW_v0 rwData = { 0 };
            rwData.EnableCollection = TRUE;
            if (SetPerTcpConnectionEStats(&tcpRow, TcpConnectionEstatsData,
                reinterpret_cast<PUCHAR>(&rwData), 0, sizeof(rwData), 0) != NO_ERROR)
            {
                return;
            }

            ULONG rodSize = sizeof(TCP_ESTATS_DATA_ROD_v0);
            std::vector<BYTE> rodBuffer(rodSize);
            PTCP_ESTATS_DATA_ROD_v0 dataRod = reinterpret_cast<PTCP_ESTATS_DATA_ROD_v0>(rodBuffer.data());

            if (GetPerTcpConnectionEStats(&tcpRow, TcpConnectionEstatsData,
                nullptr, 0, 0, nullptr, 0, 0,
                reinterpret_cast<PUCHAR>(dataRod), 0, rodSize) == NO_ERROR)
            {
                updateEnergyMetrics(dataRod);
            }
        }

        void updateEnergyMetrics(PTCP_ESTATS_DATA_ROD_v0 dataRod)
        {
            double bytesIn = static_cast<double>(dataRod->DataBytesIn);
            double bytesOut = static_cast<double>(dataRod->DataBytesOut);
            double intervalSec = interval / 1000.0;

            long downloadRate = bytesIn / intervalSec;
            long uploadRate = bytesOut / intervalSec;

            constexpr double powerConsumptionFactor = 1.138;  // Will be configurable in the future
            double downloadPower = powerConsumptionFactor * (static_cast<double>(downloadRate) / 300000);
            double uploadPower = powerConsumptionFactor * (static_cast<double>(uploadRate) / 300000);
            double averagePower = downloadPower + uploadPower;

            std::lock_guard<std::mutex> lock(dataMutex);
            auto it = std::find_if(monitoringData.begin(), monitoringData.end(),
                [](const MonitoringData& d) { return !d.getPids().empty(); });

            if (it != monitoringData.end())
            {
                it->updateNICEnergy(averagePower);
            }
        }

        void processMonitoringData()
        {
            if (newDataNic.load(std::memory_order_release))
            {
                std::unique_lock<std::mutex> lock(dataMutex);
                localMonitoringData = monitoringData;
                newDataNic.store(false, std::memory_order_release);
            }

            if (localMonitoringData.empty())
            {
                return;
            }

            TCPTableWrapper tcpWrapper;
            if (!tcpWrapper.initialize())
            {
                return;
            }

            for (const auto& data : localMonitoringData) {
                if (!data.isNICEnabled())
                {
                    continue;
                }

                for (DWORD i = 0; i < tcpWrapper.tcpTable->dwNumEntries; i++)
                {
                    for (const int& pid : data.getPids())
                    {
                        if (tcpWrapper.tcpTable->table[i].dwOwningPid == pid)
                        {
                            processConnection(tcpWrapper.tcpTable->table[i]);
                        }
                    }
                }
            }
        }

    public:
        NICMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs)
            : newDataNic(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs)
        {
        }

        void run()
        {
            while (true)
            {
                processMonitoringData();
                screen.Post(Event::Custom);

                // Reduce CPU usage by adding a small delay
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }
        }
    };
}
