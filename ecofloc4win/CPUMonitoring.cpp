#include "CPUMonitoring.h"
#include <ctime>

void CPUMonitoring::CPUMonitor::updateLocalData() 
{

    if (newData) {
        std::unique_lock<std::mutex> lock(dataMutex);
        localMonitoringData = monitoringData;
        newData.store(false, std::memory_order_release);
    }
}

CPUMonitoring::CPUMonitor::PowerMeasurement CPUMonitoring::CPUMonitor::measurePowerOverInterval() 
{
    PowerMeasurement power{};
    CPU::getCurrentPower(power.startPower);
    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    CPU::getCurrentPower(power.endPower);
    return power;
}

CPUMonitoring::CPUMonitor::TimeSnapshot CPUMonitoring::CPUMonitor::takeTimeSnapshot(const int pid) 
{
    return TimeSnapshot{
        CPU::getCPUTime(),
        CPU::getPidTime(pid)
    };
}

bool CPUMonitoring::CPUMonitor::validateTimeDifference(const TimeSnapshot& start, const TimeSnapshot& end) 
{
    double pidTimeDiff = static_cast<double>(end.pidTime - start.pidTime);
    double cpuTimeDiff = static_cast<double>(end.cpuTime - start.cpuTime);

    if (pidTimeDiff > cpuTimeDiff) {
        std::cerr << "Error: Process time is greater than CPU time." << std::endl;
        return false;
    }
    return true;
}

double CPUMonitoring::CPUMonitor::calculateCPUUsage(const TimeSnapshot& start, const TimeSnapshot& end) 
{
    double pidTimeDiff = static_cast<double>(end.pidTime - start.pidTime);
    double cpuTimeDiff = static_cast<double>(end.cpuTime - start.cpuTime);
    return pidTimeDiff / cpuTimeDiff;
}

inline double CPUMonitoring::CPUMonitor::calculateIntervalEnergy(double powerAvg, double cpuUsage) 
{
    return powerAvg * cpuUsage * interval / 1000;
}

void CPUMonitoring::CPUMonitor::updateMonitoringDataEnergy(const std::vector<int>& pids, double energy) 
{
    std::lock_guard<std::mutex> lock(dataMutex);
    auto it = std::find_if(monitoringData.begin(), monitoringData.end(),
        [&](const auto& d) { return d.getPids() == pids; });

    if (it != monitoringData.end()) 
    {
        it->updateCPUEnergy(energy);
    }
}

bool CPUMonitoring::CPUMonitor::isValidMonitoringData(const MonitoringData& data) 
{
    
    if (!data.isCPUEnabled()) 
    {
        return false;
    }
    if (data.getPids().empty()) 
    {
        std::cerr << "Error: No PIDs available for monitoring data." << std::endl;
        return false;
    }
    return true;
}

void CPUMonitoring::CPUMonitor::processMonitoringData() 
{

    std::time_t time1 = std::time(nullptr);
    double totalEnergy = 0.0;

    updateLocalData();

    for (const auto& data : localMonitoringData) 
    {
        if (!isValidMonitoringData(data)) 
        {
            continue;
        }

        const DWORD primaryPid = data.getPids()[0];
        TimeSnapshot startTime = takeTimeSnapshot(primaryPid);

        PowerMeasurement power = measurePowerOverInterval();
        TimeSnapshot endTime = takeTimeSnapshot(primaryPid);

        if (!validateTimeDifference(startTime, endTime)) 
        {
            continue;
        }

        double cpuUsage = calculateCPUUsage(startTime, endTime);
        double intervalEnergy = calculateIntervalEnergy(power.average(), cpuUsage);
        totalEnergy += intervalEnergy;

        updateMonitoringDataEnergy(data.getPids(), totalEnergy);
    }

    std::time_t time2 = std::time(nullptr);
}

void CPUMonitoring::CPUMonitor::run() 
{
    switch (this->timeout)
    {
    case -1:
        while (true) 
        {
            processMonitoringData();
            screen.Post(Event::Custom);
        }
        break;
    case 0:
        while (true) 
        {
            processMonitoringData();
        }
        break;
    default:
        int iterations = (this->timeout * 1000) / interval;
        while (iterations > 0) 
        {
            processMonitoringData();
            iterations--;
        }
        break;
    }
}
