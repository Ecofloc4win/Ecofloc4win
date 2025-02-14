#pragma once

#include <msclr/marshal_cppstd.h>
#include <iostream>
#include <ctime>
#include <fstream>

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Text::RegularExpressions;
using namespace LibreHardwareMonitor::Hardware;

/**
 * @class ManagedBridge
 * @brief Links LibreHardwareMonitor library in C# with our c++ application
 */
public ref class ManagedBridge
{
public:

    ManagedBridge()
    {
        computer = gcnew Computer();
        computer->IsCpuEnabled = true;
        computer->Open();

		String^ filename = "log.txt";

		StreamWriter^ sw = gcnew StreamWriter(filename);
		sw->WriteLine("Log file created at {0}", DateTime::Now);
		sw->WriteLine(computer->GetReport());
		sw->Close();

        lastUpdate = DateTime::Now - updateInterval;

        InitializeSensors();
    }

    /**
     * @brief Gets the global power of the CPU Cores
     * @return float The power used by all CPU Cores
     */
    float^ getCPUCoresPower()
    {
        RefreshHardware();
        return cpuPower;
    }

    /**
     * @brief Gets the list of frequencies of each CPU Cores
     * @return list of float the frequencies of each CPU Cores
     */
    List<float>^ getCPUCoresClocks()
    {
        RefreshHardware();
        return cpuClocks;
    }

    /**
     * @brief Gets the list of voltages of each CPU Cores
     * @return list of float the voltages of each CPU Cores
     */
    List<float>^ getCPUCoresVoltages()
    {
        RefreshHardware();
        return cpuVoltages;
    }

    /**
     * @brief Determines wheter the CPU is an Intel or not
     * @return bool true if is Intel's CPU and false otherwise
     */
    bool getIsIntel()
    {
        return isIntel;
    }

private:
    /**
     * @var computer
     * @brief 
     */
    Computer^ computer;

    /**
     * @var cpuPower
     * @brief Global power of the CPU Cores
     */
    float^ cpuPower = gcnew float();

    /**
     * @var cpuClocks
     * @brief List of frequencies of each CPU
     */
	List<float>^ cpuClocks = gcnew List<float>();

    /**
     * @var cpuVoltages
     * @brief List of Voltages of each CPU
     */
	List<float>^ cpuVoltages = gcnew List<float>();

    /**
     * @var powerSensors
     * @brief Contains the list of power sensors
     */
    List<ISensor^>^ powerSensors = gcnew List<ISensor^>();

    /**
     * @var clockSensors
     * @brief Contains the list of frequency sensors
     */
	List<ISensor^>^ clockSensors = gcnew List<ISensor^>();  

    /**
     * @var voltageSensors
     * @brief Contains the list of voltage sensors
     */
	List<ISensor^>^ voltageSensors = gcnew List<ISensor^>();

    /**
     * @var lastUpdate
     * @brief Date of the last update
     */
    DateTime lastUpdate;

    /**
     * @var updateInterval
     * @brief Interval for the update of sensors
     */
    TimeSpan updateInterval = TimeSpan::FromSeconds(2); // 2-second refresh interval

    /**
     * @var isIntel
     * @brief true if is intel, false otherwise
     */
	bool isIntel = true;

    /**
     * @brief Generates the same string without two first slashes
     * @param input String containing slashes
     *
     * @return String the same string in param without the two first slashes
     *         Return nullptr if it doesn't work
     */
    String^ ExtractValueBetweenSlashes(String^ input)
    {
        String^ pattern = "/([^/]+)/";
        Regex^ regex = gcnew Regex(pattern);
        Match^ match = regex->Match(input);

        if (match->Success)
        {
            return match->Groups[1]->Value;
        }
        return nullptr;
    }

    /**
     * @brief Initializes all sensors
     */
    void InitializeSensors()
    {
        for (int i = 0; i < computer->Hardware->Count; i++)
        {
            IHardware^ hardware = computer->Hardware[i];
            if (hardware->HardwareType == HardwareType::Cpu)
            {
                String^ identifier = ExtractValueBetweenSlashes(hardware->Identifier->ToString());
                hardware->Update();
                if (identifier->Contains("intel", StringComparison::OrdinalIgnoreCase))
                {
                    for (int j = 0; j < hardware->Sensors->Length; j++)
                    {
                        ISensor^ sensor = hardware->Sensors[j];
                        if (sensor->Name->Contains("CPU Cores"))
                        {
                            if (sensor->SensorType == SensorType::Power)
                            {
                                powerSensors->Add(sensor);
                            }

                        }
                    }
                }
                else
                {
                    isIntel = false;
                    for (int j = 0; j < hardware->Sensors->Length; j++)
                    {
                        ISensor^ sensor = hardware->Sensors[j];
                        if (sensor->Name->Contains("Core #"))
                        {
                            if (sensor->SensorType == SensorType::Voltage)
                            {
                                voltageSensors->Add(sensor);
                            }
                            else if (sensor->SensorType == SensorType::Clock)
                            {
                                clockSensors->Add(sensor);
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * @brief performs measurements on the CPU
     */
    void RefreshHardware()
    {
        DateTime now = DateTime::Now;
        if ((now - lastUpdate) >= updateInterval)
        {
            lastUpdate = now;

            cpuPower = 0.0f;
            if (isIntel)
            {
                for each (ISensor ^ sensor in powerSensors)
                {
                    if (sensor->Value.HasValue)
                    {
                        cpuPower = sensor->Value.Value;
                    }
                }
            }
            else
            {
                for each(ISensor ^ sensor in clockSensors)
                {
                    if (sensor->Value.HasValue)
                    {
                        cpuClocks->Add(sensor->Value.Value);
                    }
                }
                for each(ISensor ^ sensor in voltageSensors)
                {
                    if (sensor->Value.HasValue)
                    {
                        cpuVoltages->Add(sensor->Value.Value);
                    }
                }
            }
        }
    }
};

