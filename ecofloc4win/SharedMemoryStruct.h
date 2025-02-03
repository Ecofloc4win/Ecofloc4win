#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma pack(push, 1)
struct SharedEnergyData {
    double cpuEnergy;
    double gpuEnergy;
    double sdEnergy;
    double nicEnergy;
    long long timestamp;
};
#pragma pack(pop)

// Variables globales pour la mémoire partagée
extern HANDLE g_hMapFile;
extern SharedEnergyData* g_pSharedData;
#pragma once
