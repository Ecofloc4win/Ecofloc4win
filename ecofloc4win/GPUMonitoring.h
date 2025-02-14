#pragma once

#include <vector>
#include <string>
#include <windows.h>
#include <atomic>
#include <mutex>

#include "MonitoringData.h"
#include "GPU.h"

#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"

using namespace ftxui;

namespace GPUMonitoring
{
    class GPUMonitor
    {
    private:
        std::atomic<bool>& newData;
        std::mutex& dataMutex;
        std::vector<MonitoringData>& monitoringData;
        ScreenInteractive& screen;
        int interval;
        std::vector<MonitoringData> localMonitoringData;

        void processMonitoringData()
        {
            if (newData.load(std::memory_order_release))
            {
                std::unique_lock<std::mutex> lock(dataMutex);
                localMonitoringData = monitoringData;
                newData.store(false, std::memory_order_release);
            }
            for (const auto& data : localMonitoringData)
            {
                if (!data.isGPUEnabled() || data.getPids().empty())
                {
                    continue;
                }
                int gpuJoules = GPU::getGPUJoules(data.getPids(), interval);
                {
                    std::lock_guard<std::mutex> lock(dataMutex);
                    auto it = std::find_if(monitoringData.begin(), monitoringData.end(),
                        [&](const auto& d)
                        {
                            return d.getPids() == data.getPids();
                        });
                    if (it != monitoringData.end())
                    {
                        it->updateGPUEnergy(gpuJoules);
                    }
                }
            }
        }

    public:
        GPUMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs)
            : newData(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs)
        {
        }

        ~GPUMonitor() {

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

