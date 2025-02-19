#pragma once

#define NOMINMAX

#include <vector>
#include <string>
#include <windows.h>
#include <map>
#include <mutex>
#include <atomic>
#include <Pdh.h>
#include <locale>
#include <memory>
#include <stdexcept>

#include "MonitoringData.h"
#include "Utils.h"
#include "Process.h"

#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"

using namespace ftxui;

namespace StorageMonitoring {
    // Helper class for registry operations
    class RegistryHelper {
    public:
        /**
         * @brief Gets the index of a counter in the registry based on its name.
         *
         * @param counterName The name of the counter
         * @return DWORD the Index of the counter
         */
        static DWORD getCounterIndex(const std::string& counterName);

    private:
        class AutoRegistryKey {
        public:
            AutoRegistryKey(const wchar_t* path) {
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
                    hKey = nullptr;
                }
            }
            ~AutoRegistryKey() {
                if (hKey) RegCloseKey(hKey);
            }
            HKEY get() const { return hKey; }
            bool isValid() const { return hKey != nullptr; }
        private:
            HKEY hKey;
        };

        static DWORD parseCounterData(HKEY hKey, const std::string& counterName);

        static DWORD findCounterIndex(const char* buffer, DWORD size, const std::string& targetName);
    };

    // Helper class for PDH operations
    class PDHHelper {
    public:
        /**
         * @brief Gets the localized counter path for a given process name and counter name to be used in PDH functions
         * 		  and avoid hardcoding the counter path for each language.
         *
         * @param processName The name of the process
         * @param counterName The name of the counter
         * @return wstring the localized counter path for the process
         */
        static std::wstring getLocalizedCounterPath(const std::wstring& processName, const std::string& counterName);

        static std::wstring getInstanceForPID(int targetPID);

    private:
        class PDHQueryWrapper {
        public:
            PDHQueryWrapper() {
                if (PdhOpenQuery(nullptr, 0, &query) != ERROR_SUCCESS) {
                    throw std::runtime_error("Failed to open PDH query");
                }
            }
            ~PDHQueryWrapper() { PdhCloseQuery(query); }
            PDH_HQUERY get() const { return query; }
        private:
            PDH_HQUERY query;
        };

        static bool lookupPerfName(DWORD index, wchar_t* buffer, DWORD size);

        static std::wstring buildCounterPath(const wchar_t* processName, const std::wstring& instanceName, const wchar_t* counterName);

        /**
         * @brief Gets the name of the instance for a given process ID.
         *
         * @param targetPID The pid
         * @return wstring the name of the instance
         */
        static std::wstring findInstanceForPID(PDH_HCOUNTER pidCounter, int targetPID);
    };

    class SDMonitor {
    private:
        std::atomic<bool>& newDataSd;
        std::mutex& dataMutex;
        std::vector<MonitoringData>& monitoringData;
		std::vector<MonitoringData> localMonitoringData;
        ScreenInteractive& screen;
        int interval;
        int timeout;
        PDH_HQUERY query;
        std::map<std::wstring, std::pair<PDH_HCOUNTER, PDH_HCOUNTER>> processCounters;

        bool initializeQuery();

        bool setupCounters(const std::wstring& instanceName);

        void collectAndUpdateMetrics();

        void updateEnergyMetrics(long readRate, long writeRate);

        void processMonitoringData();

    public:
        SDMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
        std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs)
        : newDataSd(newDataFlag), dataMutex(mutex), monitoringData(data),
        screen(scr), interval(intervalMs), timeout(-1)
        {
            if (!initializeQuery()) {
                throw std::runtime_error("Failed to open PDH query.");
            }
        }

        SDMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs, int timeout)
            : newDataSd(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs), timeout(timeout)
        {
            if (!initializeQuery()) {
                throw std::runtime_error("Failed to open PDH query.");
            }
        }

        ~SDMonitor() {
            PdhCloseQuery(query);
        }

        void run();
    };
}