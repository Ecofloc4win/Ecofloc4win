#include "pch.h"

#include "Wrapper.h"
#include <vector>

extern "C" __declspec(dllexport) float* getCPUCoresPower(int* size) {
	ManagedBridge^ bridge = gcnew ManagedBridge();
	float^ power = bridge->getCPUCoresPower();
	delete bridge;
	
	*size = 1;
	float* powerArray = new float[*size];
	powerArray[0] = *power;

	return powerArray;
}

extern "C" __declspec(dllexport) float* getCPUCoresClocks(int* size) {
	ManagedBridge^ bridge = gcnew ManagedBridge();
	List<float>^ clocks = bridge->getCPUCoresClocks();
	delete bridge;

	*size = clocks->Count;
	float* clocksArray = new float[*size];

	for (int i = 0; i < *size; i++) {
		clocksArray[i] = clocks[i];
	}

	return clocksArray;
}

extern "C" __declspec(dllexport) float* getCPUCoresVoltages(int* size) {
	ManagedBridge^ bridge = gcnew ManagedBridge();
	List<float>^ voltages = bridge->getCPUCoresVoltages();
	delete bridge;

	*size = voltages->Count;
	float* voltagesArray = new float[*size];

	for (int i = 0; i < *size; i++) {
		voltagesArray[i] = voltages[i];
	}

	return voltagesArray;
}