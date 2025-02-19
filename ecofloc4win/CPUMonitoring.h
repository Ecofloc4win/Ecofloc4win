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

namespace CPUMonitoring {
    class CPUMonitor {
    private:
        std::atomic<bool>& newData;
        std::mutex& dataMutex;
        std::vector<MonitoringData>& monitoringData;
        ScreenInteractive& screen;
        int interval;
        int timeout;
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

        void updateLocalData();

        PowerMeasurement measurePowerOverInterval();

        TimeSnapshot takeTimeSnapshot(const int pid);

        bool validateTimeDifference(const TimeSnapshot& start, const TimeSnapshot& end);

        double calculateCPUUsage(const TimeSnapshot& start, const TimeSnapshot& end);

        double calculateIntervalEnergy(double powerAvg, double cpuUsage);

        void updateMonitoringDataEnergy(const std::vector<int>& pids, double energy);

        bool isValidMonitoringData(const MonitoringData& data);

        void processMonitoringData();

    public:
        CPUMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs)
            : newData(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs), timeout(-1)
        {
        }

        CPUMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs, int timeout)
            : newData(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs), timeout(timeout)
        {
        }

        ~CPUMonitor() = default;

        void run();
    };
}