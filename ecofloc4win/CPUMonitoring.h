#pragma once

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

namespace CPUMonitoring
{
    class CPUMonitor
    {
    private:
        std::atomic<bool>& newData;
        std::mutex& dataMutex;
        std::vector<MonitoringData>& monitoringData;
        ScreenInteractive& screen;
        int interval;
        std::vector<MonitoringData> localMonitoringData;



        void processMonitoringData() {
			double totalEnergy = 0.0;
			double startTotalPower = 0.0;
			double endTotalPower = 0.0;
			double avgPowerInterval = 0.0;

			if (newData)
			{
				std::unique_lock<std::mutex> lock(dataMutex);
				localMonitoringData = monitoringData;
				newData.store(false, std::memory_order_release);
			}

			// Process each monitoring data entry
			for (auto& data : localMonitoringData)
			{
				if (!data.isCPUEnabled())
				{
					continue;
				}

				// Ensure the PID list is not empty
				if (data.getPids().empty())
				{
					std::cerr << "Error: No PIDs available for monitoring data." << std::endl;
					continue;
				}

				// Get initial CPU and process times
				uint64_t startCPUTime = CPU::getCPUTime();
				uint64_t startPidTime = CPU::getPidTime(data.getPids()[0]);

				CPU::getCurrentPower(startTotalPower);

				// Monitor for the specified interval
				std::this_thread::sleep_for(std::chrono::milliseconds(interval));

				CPU::getCurrentPower(endTotalPower);

				avgPowerInterval = (startTotalPower + endTotalPower) / 2;

				uint64_t endCPUTime = CPU::getCPUTime();
				uint64_t endPidTime = CPU::getPidTime(data.getPids()[0]);

				// Calculate time differences
				double pidTimeDiff = static_cast<double>(endPidTime) - static_cast<double>(startPidTime);
				double cpuTimeDiff = static_cast<double>(endCPUTime) - static_cast<double>(startCPUTime);

				// Validate time differences
				if (pidTimeDiff > cpuTimeDiff)
				{
					std::cerr << "Error: Process time is greater than CPU time." << std::endl;
					continue;
				}

				// Calculate CPU usage and energy consumption
				double cpuUsage = (pidTimeDiff / cpuTimeDiff);
				double intervalEnergy = avgPowerInterval * cpuUsage * interval / 1000;

				totalEnergy += intervalEnergy;

				// Update monitoring data safely
				{
					std::lock_guard<std::mutex> lock(dataMutex);
					auto it = std::find_if(monitoringData.begin(), monitoringData.end(),
						[&](const auto& d)
						{
							return d.getPids() == data.getPids();
						});

					if (it != monitoringData.end())
					{
						it->updateCPUEnergy(totalEnergy);
					}
				}
			}
        }
        
    public:
        CPUMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs)
            : newData(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs)
        {
        }

        ~CPUMonitor() {

        }

        void run() {
            while (true) {
                processMonitoringData();
                screen.Post(Event::Custom);

                // Reduce CPU usage by adding a small delay
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }
        }
    };
};

