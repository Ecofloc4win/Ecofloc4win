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

/**
* @brief Namespace for GPU monitoring related classes and functions.
*/
namespace GPUMonitoring
{
	/**
	* @brief Class to monitor GPU devices.
	*/
    class GPUMonitor
    {
    private:
        std::atomic<bool>& newData;
        std::mutex& dataMutex;
        std::vector<MonitoringData>& monitoringData;
        ScreenInteractive& screen;
        int interval;
        std::vector<MonitoringData> localMonitoringData;

		/**
		* @brief Processes the monitoring data for the GPU devices.
		*/
        void processMonitoringData();

    public:
		/**
		* @brief Constructor for the GPUMonitor class.
        */
        GPUMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs)
            : newData(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs)
        {
        }

        /**
		* @brief Destructor for the GPUMonitor class.
        */
        ~GPUMonitor() {

        }

        /**
        * @brief Runs the GPU monitoring process.
        */
        void run();
    };
};

