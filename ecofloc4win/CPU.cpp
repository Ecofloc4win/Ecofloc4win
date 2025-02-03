/**
 * @file CPU.cpp
 * @brief Definition of CPU management features.
 * @author Ecofloc's Team
 * @date 2025-02-03
 */

#include "CPU.h"

/**
 * @brief Typedef for a function pointer to retrieve CPU voltages.
 */
typedef float* (*get_cpu_voltages_func)(int* size);

/**
 * @brief Typedef for a function pointer to retrieve CPU clocks.
 */
typedef float* (*get_cpu_clocks_func)(int* size);

/**
 * @brief Typedef for a function pointer to retrieve CPU cores' power.
 */
typedef float* (*get_cpu_cores_power_func)(int* size);

/**
 * @namespace CPU
 * @brief Namespace for CPU-related functionalities.
 */
namespace CPU
{
    uint64_t fromFileTime(const FILETIME& ft)
    {
        ULARGE_INTEGER uli = { 0 };
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        return uli.QuadPart;
    }

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

    bool getCurrentPower(double& power)
    {
        HMODULE h_module = LoadLibrary(L"Wrapper.dll");
        if (!h_module)
        {
            std::cerr << "Failed to load Wrapper.dll. Error code: " << GetLastError() << std::endl;
            return false;
        }

        get_cpu_cores_power_func get_cpu_cores_power = (get_cpu_cores_power_func)GetProcAddress(h_module, "getCPUCoresPower");
        if (!get_cpu_cores_power)
        {
            std::cerr << "Failed to get function address for getCPUCoresPower. Error code: " << GetLastError() << std::endl;
            FreeLibrary(h_module);
            return false;
        }

        int power_size = 0;
        int cpu_count = 0;
        float* power_array = get_cpu_cores_power(&power_size);

        if (power_array)
        {
            for (int i = 0; i < power_size; i++)
            {
                power += power_array[i];
                cpu_count++;
            }
            delete[] power_array;
        }
        else
        {
            std::cerr << "Failed to retrieve CPU power." << std::endl;
        }

        if (cpu_count > 0)
        {
            power = power / cpu_count;
        }
        else
        {
            power = 0.0;
        }

        FreeLibrary(h_module);

        return true;
    }
}
