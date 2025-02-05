#include "CPU.h"
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;

/// Typedef for a function pointer to retrieve CPU voltages.
typedef float* (*get_cpu_voltages_func)(int* size);

/// Typedef for a function pointer to retrieve CPU clocks.
typedef float* (*get_cpu_clocks_func)(int* size);

/// Typedef for a function pointer to retrieve CPU power.
typedef float* (*get_cpu_power_func)(int* size);

/// Typedef for a function pointer to retrieve CPU power.
typedef bool (*get_is_intel_cpu_func)();

/// Namespace for CPU-related functionalities.
namespace CPU
{
    /**
     * @brief Give the capacitance of the cpu.
     *
     * @return double The CPU capacitance. -1.0 on failure.
     */
    double getCapacitance() {
        try {
            std::ifstream f("../Config/cpu.json");
            if (!f.is_open()) {
                throw std::runtime_error("Could not open ../Config/cpu.json, make sure it exists");
            }
            json cpu_data = json::parse(f);

            const double CPU_TDP = cpu_data["tdp"];
            const double CPU_FREQ_TDP = cpu_data["clockSpeed"];
            const double CPU_VOLT_TDP = cpu_data["voltage"];

            double capacitance = (0.7 * CPU_TDP) / (CPU_FREQ_TDP * CPU_VOLT_TDP * CPU_VOLT_TDP);

            return capacitance;
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading capacitance data: " << e.what() << std::endl;
            return -1.0;
        }
    }

    /**
     * @brief Converts a FILETIME structure to a uint64_t.
     *
     * @param ft The FILETIME structure to convert.
     * @return uint64_t The converted value.
     */
    uint64_t fromFileTime(const FILETIME& ft)
    {
        ULARGE_INTEGER uli = { 0 };
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        return uli.QuadPart;
    }

    /**
     * @brief Retrieves the total CPU time including idle, kernel, and user times.
     *
     * @return uint64_t The total CPU time in 100-nanosecond intervals. Returns -1 on failure.
     */
    uint64_t getCPUTime()
    {
        FILETIME idle_time, kernel_time, user_time;

        if (GetSystemTimes(&idle_time, &kernel_time, &user_time))
        {
            uint64_t cpu_time = fromFileTime(kernel_time) + fromFileTime(user_time) + fromFileTime(idle_time);
            return cpu_time;
        }
        else
        {
            std::cerr << "Failed to get CPU Time: Error " << GetLastError() << std::endl;
            return -1;
        }
    }

    /**
     * @brief Retrieves the total time spent by a process.
     *
     * @param pid The process ID of the target process.
     * @return uint64_t The total process time (kernel + user) in 100-nanosecond intervals. Returns -1 on failure.
     */
    uint64_t getPidTime(DWORD pid)
    {
        HANDLE h_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (h_process == NULL)
        {
            std::cerr << "Failed to open process handle. Error: " << GetLastError() << std::endl;
            return -1;
        }

        FILETIME creation_time, exit_time, kernel_time, user_time;

        if (GetProcessTimes(h_process, &creation_time, &exit_time, &kernel_time, &user_time))
        {
            uint64_t kernel = fromFileTime(kernel_time);
            uint64_t user = fromFileTime(user_time);

            uint64_t total_time = kernel + user;
            CloseHandle(h_process);
            return total_time;
        }
        else
        {
            std::cerr << "Failed to get process times. Error: " << GetLastError() << std::endl;
            CloseHandle(h_process);
            return -1;
        }
    }


    bool getAvgFreq(double& freq)
    {

        HMODULE h_module = LoadLibrary(L"Wrapper.dll");
        if (!h_module)
        {
            std::cerr << "Failed to load Wrapper.dll. Error code: " << GetLastError() << std::endl;
            return false;
        }

        get_cpu_clocks_func get_cpu_cores_clock = (get_cpu_clocks_func)GetProcAddress(h_module, "getCPUCoresClocks");
        if (!get_cpu_cores_clock)
        {
            std::cerr << "Failed to get function address for getCPUClockSpeed. Error code: " << GetLastError() << std::endl;
            FreeLibrary(h_module);
            return false;
        }

        int clock_size = 0;
        float* clock_array = get_cpu_cores_clock(&clock_size);

        if (!clock_array)
        {
            std::cerr << "Failed to retrieve CPU frequence." << std::endl;
            FreeLibrary(h_module);
            return false;
        }

        // Initialize freq and cpu_count
        freq = 0.0;
        int cpu_count = 0;

        for (int i = 0; i < clock_size; i++)
        {
            freq += clock_array[i];
            //std::cout << "CPU CLOCK[" << i << "] : " << clock_array[i] << std::endl;
            cpu_count++;
        }

        delete[] clock_array;  // Free the allocated memory for the freq array

        if (cpu_count > 0)
        {
            freq /= cpu_count;  // Compute average freq
        }
        else
        {
            freq = 0.0;  // If no valid CPU frequence, set freq to 0
            return false;
        }

        FreeLibrary(h_module);  // Free the loaded library
        return true;
    }

    bool getAvgVolt(double& volt)
    {
        HMODULE h_module = LoadLibrary(L"Wrapper.dll");
        if (!h_module)
        {
            std::cerr << "Failed to load Wrapper.dll. Error code: " << GetLastError() << std::endl;
            return false;
        }

        get_cpu_voltages_func get_cpu_cores_voltage = (get_cpu_voltages_func)GetProcAddress(h_module, "getCPUCoresVoltages");
        if (!get_cpu_cores_voltage)
        {
            std::cerr << "Failed to get function address for getCPUVoltage. Error code: " << GetLastError() << std::endl;
            FreeLibrary(h_module);
            return false;
        }

        int volt_size = 0;
        float* volt_array = get_cpu_cores_voltage(&volt_size);

        //std::cout << "Volt array size : " << volt_size << std::endl;
        //std::cout << "Volt array[0] : " << volt_array[0] << std::endl;

        if (!volt_array)
        {
            std::cerr << "Failed to retrieve CPU voltage." << std::endl;
            FreeLibrary(h_module);
            return false;
        }

        // Initialize volt and cpu_count
        volt = 0.0;
        int cpu_count = 0;

        for (int i = 0; i < volt_size; i++)
        {
            volt += volt_array[i];
            //std::cout << "CPU VOLT[" << i << "] : " << volt_array[i] << std::endl;
            cpu_count++;
        }

        delete[] volt_array;  // Free the allocated memory for the volt array

        if (cpu_count > 0)
        {
            volt /= cpu_count;  // Compute average volt
        }
        else
        {
            volt = 0.0;  // If no valid CPU volt, set volt to 0
            return false;
        }

        FreeLibrary(h_module);  // Free the loaded library
        return true;
    }

    /**
     * @brief Retrieves the current power consumption of the CPU.
     *
     * This function uses a DLL (`Wrapper.dll`) to retrieve the power consumption of CPU cores.
     *
     * @param power A reference to a double where the calculated power will be stored.
     * @return bool True if the power was successfully retrieved, false otherwise.
     */
    bool getCurrentPower(double& power)
    {
        HMODULE h_module = LoadLibrary(L"Wrapper.dll");
        if (!h_module)
        {
            std::cerr << "Failed to load Wrapper.dll. Error code: " << GetLastError() << std::endl;
            return false;
        }

        get_is_intel_cpu_func get_is_intel = (get_is_intel_cpu_func)GetProcAddress(h_module, "getIsIntel");
        if (!get_is_intel)
        {
            std::cerr << "Failed to get function address for getCPUClockSpeed. Error code: " << GetLastError() << std::endl;
            FreeLibrary(h_module);
            return false;
        }

        if (!get_is_intel())
        {
            double capacitance = getCapacitance();
            double avg_freq = 0.0;
            double avg_volt = 0.0;

            if (!getAvgFreq(avg_freq))
            {
                std::cerr << "Error while attempting to get the cpu frequence";
                return false;
            }
            //std::cout << "Avg Freq : " << avg_freq << std::endl;

            if (!getAvgVolt(avg_volt))
            {
                std::cerr << "Error while attempting to get the cpu voltage";
                return false;
            }
            //std::cout << "Avg volt : " << avg_volt << std::endl;


            //std::cout << "capacitance : " << capacitance << std::endl;
            power = capacitance * avg_freq * avg_volt * avg_volt;
            //std::cout << "power : " << power << std::endl;
        }
        else
        {
            get_cpu_power_func get_cpu_power = (get_cpu_power_func)GetProcAddress(h_module, "getCPUCoresPower");
            if (!get_cpu_power)
            {
                std::cerr << "Failed to get function address for getCPUClockSpeed. Error code: " << GetLastError() << std::endl;
                FreeLibrary(h_module);
                return false;
            }

            int power_size = 0;
            float* power_array = get_cpu_power(&power_size);

            if (!power_array)
            {
                std::cerr << "Failed to retrieve CPU frequence." << std::endl;
                FreeLibrary(h_module);
                return false;
            }

            // Initialize power and cpu_count
            power = 0.0;
            int cpu_count = 0;

            for (int i = 0; i < power_size; i++)
            {
                power += power_array[i];
                //std::cout << "CPU POWER[" << i + 1 << "] : " << power_array[i] << std::endl;
                cpu_count++;
            }

            delete[] power_array;  // Free the allocated memory for the power array

            if (cpu_count > 0)
            {
                power /= cpu_count;  // Compute average power
            }
            else
            {
                power = 0.0;  // If no valid CPU power, set power to 0
                return false;
            }

        }

        return true;

    }

}