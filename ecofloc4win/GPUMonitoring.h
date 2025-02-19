#pragma once

#define NOMINMAX

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
        int timeout;
        std::vector<MonitoringData> localMonitoringData;

        void processMonitoringData();

    public:
        GPUMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs)
            : newData(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs), timeout(-1)
        {
        }

        GPUMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs, int timeout)
            : newData(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs), timeout(timeout)
        {
        }

        ~GPUMonitor() {

        }

        void run();
    };
};

