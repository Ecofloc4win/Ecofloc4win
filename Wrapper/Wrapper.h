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

public ref class ManagedBridge
{
public:
    ManagedBridge() {
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

    float^ getCPUCoresPower() {
        RefreshHardware();
        return cpuPower;
    }

    List<float>^ getCPUCoresClocks() {
        RefreshHardware();
        return cpuClocks;
    }

    List<float>^ getCPUCoresVoltages() {
        RefreshHardware();
        return cpuVoltages;
    }

    bool getIsIntel()
    {
        return isIntel;
    }

private:
    Computer^ computer;
    float^ cpuPower = gcnew float();
	List<float>^ cpuClocks = gcnew List<float>();
	List<float>^ cpuVoltages = gcnew List<float>();
    List<ISensor^>^ powerSensors = gcnew List<ISensor^>();
	List<ISensor^>^ clockSensors = gcnew List<ISensor^>();  
	List<ISensor^>^ voltageSensors = gcnew List<ISensor^>();
    DateTime lastUpdate;
    TimeSpan updateInterval = TimeSpan::FromSeconds(2); // 2-second refresh interval
	bool isIntel = true;

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

    void InitializeSensors() {
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
                else {
                    isIntel = false;
                    for (int j = 0; j < hardware->Sensors->Length; j++)
                    {
                        ISensor^ sensor = hardware->Sensors[j];
                        if (sensor->Name->Contains("Core #")) {
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
                    if (sensor->Value.HasValue) {
                        cpuPower = sensor->Value.Value;
                    }
                }
            }
            else
            {
                for each(ISensor ^ sensor in clockSensors)
                {
                    if (sensor->Value.HasValue) {
                        cpuClocks->Add(sensor->Value.Value);
                    }
                }
                for each(ISensor ^ sensor in voltageSensors)
                {
                    if (sensor->Value.HasValue) {
                        cpuVoltages->Add(sensor->Value.Value);
                    }
                }
            }
        }
    }
};

