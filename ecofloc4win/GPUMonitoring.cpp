#include "GPUMonitoring.h"

void GPUMonitoring::GPUMonitor::processMonitoringData()
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

void GPUMonitoring::GPUMonitor::run() 
{
    switch (this->timeout)
    {
    case -1:
        while (true)
        {
            processMonitoringData();
            screen.Post(Event::Custom);
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
        break;
    case 0:
        while (true)
        {
            processMonitoringData();
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
        break;
    default:
        int iterations = (this->timeout * 1000) / interval;
        while (iterations > 0)
        {
            processMonitoringData();
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            iterations--;
        }
        break;
    }
}
