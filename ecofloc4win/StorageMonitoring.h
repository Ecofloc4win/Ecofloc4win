#pragma once

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
        static DWORD getCounterIndex(const std::string& counterName) {
            HKEY hKey;
            const wchar_t* registryPath = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\009";

            AutoRegistryKey key(registryPath);
            if (!key.isValid()) {
                throw std::runtime_error("Failed to open registry key");
            }

            return parseCounterData(key.get(), counterName);
        }

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

        static DWORD parseCounterData(HKEY hKey, const std::string& counterName) {
            DWORD dataSize = 0;
            if (RegQueryValueEx(hKey, L"Counter", NULL, NULL, NULL, &dataSize) != ERROR_SUCCESS) {
                throw std::runtime_error("Failed to query registry value size");
            }

            std::vector<char> buffer(dataSize);
            if (RegQueryValueEx(hKey, L"Counter", NULL, NULL, reinterpret_cast<LPBYTE>(buffer.data()), &dataSize) != ERROR_SUCCESS) {
                throw std::runtime_error("Failed to query registry value");
            }

            return findCounterIndex(buffer.data(), dataSize, counterName);
        }

        static DWORD findCounterIndex(const char* buffer, DWORD size, const std::string& targetName) {
            std::string temp;
            std::string currentIndex;
            bool isIndex = true;

            for (DWORD i = 0; i < size; i += 2) {
                if (buffer[i] == '\0') {
                    if (!temp.empty()) {
                        if (isIndex) {
                            currentIndex = temp;
                        }
                        else if (temp == targetName) {
                            return std::stoul(currentIndex);
                        }
                        temp.clear();
                        isIndex = !isIndex;
                    }
                }
                else {
                    temp += buffer[i];
                }
            }
            return -1;
        }
    };

    // Helper class for PDH operations
    class PDHHelper {
    public:
        static std::wstring getLocalizedCounterPath(const std::wstring& processName, const std::string& counterName) {
            wchar_t localizedName[PDH_MAX_COUNTER_PATH];
            wchar_t localizedProcessName[PDH_MAX_COUNTER_PATH];
            DWORD size = PDH_MAX_COUNTER_PATH;

            DWORD counterIndex = RegistryHelper::getCounterIndex(counterName);
            DWORD processIndex = RegistryHelper::getCounterIndex("Process");

            if (!lookupPerfName(counterIndex, localizedName, size) ||
                !lookupPerfName(processIndex, localizedProcessName, size)) {
                throw std::runtime_error("Failed to get localized counter path");
            }

            return buildCounterPath(localizedProcessName, processName, localizedName);
        }

        static std::wstring getInstanceForPID(int targetPID) {
            PDHQueryWrapper query;
            std::wstring queryPath = getLocalizedCounterPath(L"*", "ID Process");

            PDH_HCOUNTER pidCounter;
            if (PdhAddCounter(query.get(), queryPath.c_str(), 0, &pidCounter) != ERROR_SUCCESS) {
                throw std::runtime_error("Failed to add counter for process ID");
            }

            if (PdhCollectQueryData(query.get()) != ERROR_SUCCESS) {
                throw std::runtime_error("Failed to collect query data");
            }

            return findInstanceForPID(pidCounter, targetPID);
        }

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

        static bool lookupPerfName(DWORD index, wchar_t* buffer, DWORD size) {
            return PdhLookupPerfNameByIndex(NULL, index, buffer, &size) == ERROR_SUCCESS;
        }

        static std::wstring buildCounterPath(const wchar_t* processName, const std::wstring& instanceName, const wchar_t* counterName) {
            return L"\\" + std::wstring(processName) + L"(" + instanceName + L")\\" + std::wstring(counterName);
        }

        static std::wstring findInstanceForPID(PDH_HCOUNTER pidCounter, int targetPID) {
            DWORD bufferSize = 0;
            DWORD itemCount = 0;
            PdhGetRawCounterArray(pidCounter, &bufferSize, &itemCount, nullptr);

            std::vector<BYTE> buffer(bufferSize);
            PDH_RAW_COUNTER_ITEM* items = reinterpret_cast<PDH_RAW_COUNTER_ITEM*>(buffer.data());

            if (PdhGetRawCounterArray(pidCounter, &bufferSize, &itemCount, items) != ERROR_SUCCESS) {
                throw std::runtime_error("Failed to get counter array");
            }

            for (DWORD i = 0; i < itemCount; ++i) {
                if (static_cast<int>(items[i].RawValue.FirstValue) == targetPID) {
                    return items[i].szName;
                }
            }
            return L"";
        }
    };

    class SDMonitor {
    private:
        std::atomic<bool>& newDataSd;
        std::mutex& dataMutex;
        std::vector<MonitoringData>& monitoringData;
		std::vector<MonitoringData> localMonitoringData;
        ScreenInteractive& screen;
        int interval;
        PDH_HQUERY query;
        std::map<std::wstring, std::pair<PDH_HCOUNTER, PDH_HCOUNTER>> processCounters;

        bool initializeQuery() {
            return PdhOpenQuery(NULL, 0, &query) == ERROR_SUCCESS;
        }

        bool setupCounters(const std::wstring& instanceName) {
            if (processCounters.find(instanceName) != processCounters.end()) {
                return true;
            }

            try {
                PDH_HCOUNTER counterDiskRead, counterDiskWrite;
                std::wstring readPath = PDHHelper::getLocalizedCounterPath(instanceName, "IO Read Bytes/sec");
                std::wstring writePath = PDHHelper::getLocalizedCounterPath(instanceName, "IO Write Bytes/sec");

                if (PdhAddCounter(query, readPath.c_str(), 0, &counterDiskRead) != ERROR_SUCCESS ||
                    PdhAddCounter(query, writePath.c_str(), 0, &counterDiskWrite) != ERROR_SUCCESS) {
                    return false;
                }

                processCounters[instanceName] = { counterDiskRead, counterDiskWrite };
                return true;
            }
            catch (const std::exception& e) {
                std::cerr << "Error setting up counters: " << e.what() << std::endl;
                return false;
            }
        }

        void collectAndUpdateMetrics() {
            if (PdhCollectQueryData(query) != ERROR_SUCCESS) {
                throw std::runtime_error("Failed to collect PDH query data.");
            }

            for (const auto& [instanceName, counters] : processCounters) {
                PDH_FMT_COUNTERVALUE diskReadValue, diskWriteValue;
                long readRate = 0, writeRate = 0;

                if (PdhGetFormattedCounterValue(counters.first, PDH_FMT_LONG, NULL, &diskReadValue) == ERROR_SUCCESS) {
                    readRate = diskReadValue.longValue;
                }
                if (PdhGetFormattedCounterValue(counters.second, PDH_FMT_LONG, NULL, &diskWriteValue) == ERROR_SUCCESS) {
                    writeRate = diskWriteValue.longValue;
                }

                updateEnergyMetrics(readRate, writeRate);
            }
        }

        void updateEnergyMetrics(long readRate, long writeRate) {
            constexpr double readPowerFactor = 2.2 / 5600000000.0;
            constexpr double writePowerFactor = 2.2 / 5300000000.0;

            double readPower = readPowerFactor * readRate;
            double writePower = writePowerFactor * writeRate;
            double averagePower = readPower + writePower;
            double intervalEnergy = averagePower * interval / 1000.0;

            std::lock_guard<std::mutex> lock(dataMutex);
            auto it = std::find_if(monitoringData.begin(), monitoringData.end(),
                [](const auto& d) { return !d.getPids().empty(); });

            if (it != monitoringData.end()) {
                it->updateSDEnergy(intervalEnergy);
            }
        }

        void processMonitoringData() {
            if (newDataSd.load(std::memory_order_release)) {
                std::unique_lock<std::mutex> lock(dataMutex);
                localMonitoringData = monitoringData;
                newDataSd.store(false, std::memory_order_release);
            }

            for (const auto& data : localMonitoringData) {
                if (!data.isSDEnabled() || data.getPids().empty()) {
                    continue;
                }

                try {
                    std::wstring instanceName = PDHHelper::getInstanceForPID(data.getPids()[0]);
                    if (instanceName.empty() || !setupCounters(instanceName)) {
                        continue;
                    }

                    collectAndUpdateMetrics();
                }
                catch (const std::exception& e) {
                    std::cerr << "Error processing monitoring data: " << e.what() << std::endl;
                }
            }
        }

    public:
        SDMonitor(std::atomic<bool>& newDataFlag, std::mutex& mutex,
            std::vector<MonitoringData>& data, ScreenInteractive& scr, int intervalMs)
            : newDataSd(newDataFlag), dataMutex(mutex), monitoringData(data),
            screen(scr), interval(intervalMs) {
            if (!initializeQuery()) {
                throw std::runtime_error("Failed to open PDH query.");
            }
        }

        ~SDMonitor() {
            PdhCloseQuery(query);
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
}