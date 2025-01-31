#pragma once

#include <msclr/marshal_cppstd.h>
#include <iostream>
#include <ctime>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Text::RegularExpressions;
using namespace LibreHardwareMonitor::Hardware;

public ref class ManagedBridge
{
public:
    ManagedBridge() {
        try {
            computer = gcnew Computer();
            computer->IsCpuEnabled = true;
            computer->Open();
            lastUpdate = DateTime::Now - updateInterval;

            InitializeSensors();
        }
        catch (Exception^ ex) {
            Console::WriteLine("Error in ManagedBridge constructor: " + ex->Message);
        }
    }

    float^ getCPUCoresPower() {
        try {
            RefreshHardware();
            return cpuPower;
        }
        catch (Exception^ ex) {
            Console::WriteLine("Error in getCPUCoresPower: " + ex->Message);
            return nullptr;
        }
    }

private:
    Computer^ computer;
    float^ cpuPower = gcnew float();
    List<ISensor^>^ powerSensors = gcnew List<ISensor^>();
	List<ISensor^>^ clockSensors = gcnew List<ISensor^>();  
	List<ISensor^>^ voltageSensors = gcnew List<ISensor^>();
    DateTime lastUpdate;
    TimeSpan updateInterval = TimeSpan::FromSeconds(2); // 2-second refresh interval

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
                    for (int j = 0; j < hardware->Sensors->Length; j++)
                    {
                        ISensor^ sensor = hardware->Sensors[j];
                        if (sensor->Name->Contains("Core #")) {
                            if (sensor->SensorType == SensorType::Voltage)
                            {
                                powerSensors->Add(sensor);
                            }
                            else if (sensor->SensorType == SensorType::Clock)
                            {
                                clockSensors->Add(sensor);
                            }
                        }
                    }
                    Console::WriteLine(clockSensors->Count);
                    Console::WriteLine(powerSensors->Count);
                }

            }
        }
    }

    void RefreshHardware() {
        DateTime now = DateTime::Now;
        if ((now - lastUpdate) >= updateInterval) {
            lastUpdate = now;

            cpuPower = 0.0f;

            for each (ISensor ^ sensor in powerSensors) {
                if (sensor->Value.HasValue) {
                    cpuPower = sensor->Value.Value;
                }
            }
        }
    }
};

