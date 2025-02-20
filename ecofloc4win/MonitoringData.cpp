/**
 * @file MonitoringData.cpp
 * @brief Definition of process management features.
 * @author Ecofloc's Team
 * @date 2025-02-03
 */

#include "MonitoringData.h"
#include "Utils.h"

#include <unordered_map>
#include <stdexcept>

/**
 * @brief Returns the Component as an object based on a string
 *
 * @param str the name of the component wanted as a string
 * @return ComponentType The component wanted as an object
 * @throw std::invalid_argument If str is not CPU, GPU, SD or NIC
 */
Utils::ComponentType stringToComponentType(const std::string& str)
{
	auto it = Utils::componentMap.find(str);
	if (it != Utils::componentMap.end())
	{
		return it->second;
	}

	throw std::invalid_argument("Invalid component type: " + str);
}

std::string MonitoringData::getName() const
{
	return name;
}

std::vector<int> MonitoringData::getPids() const
{
	return pids;
}

void MonitoringData::enableComponent(const std::string& componentStr)
{
	Utils::ComponentType component = stringToComponentType(componentStr);

	switch (component)
	{
	case Utils::CPU:
		cpuEnabled = true;
		break;
	case Utils::GPU:
		gpuEnabled = true;
		break;
	case Utils::SD:
		sdEnabled = true;
		break;
	case Utils::NIC:
		nicEnabled = true;
		break;
	}
}

void MonitoringData::disableComponent(const std::string& componentStr)
{
	Utils::ComponentType component = stringToComponentType(componentStr);
	switch (component)
	{
	case Utils::CPU:
		cpuEnabled = false;
		break;
	case Utils::GPU:
		gpuEnabled = false;
		break;
	case Utils::SD:
		sdEnabled = false;
		break;
	case Utils::NIC:
		nicEnabled = false;
		break;
	}
}

bool MonitoringData::isCPUEnabled() const
{
	return cpuEnabled;
}

bool MonitoringData::isGPUEnabled() const
{
	return gpuEnabled;
}

bool MonitoringData::isSDEnabled() const
{
	return sdEnabled;
}

bool MonitoringData::isNICEnabled() const
{
	return nicEnabled;
}

double MonitoringData::getCPUEnergy() const
{
	return cpuEnergy;
}

double MonitoringData::getAvgCPUEnergy() const
{
	return avgCpuEnergy;
}

double MonitoringData::getCurrentCPUPower() const
{
	return currentCpuPower;
}

double MonitoringData::getGPUEnergy() const
{
	return gpuEnergy;
}

double MonitoringData::getAvgGPUEnergy() const
{
	return avgGpuEnergy;
}

double MonitoringData::getCurrentGPUPower() const
{
	return currentGpuPower;
}

double MonitoringData::getSDEnergy() const
{
	return sdEnergy;
}

double MonitoringData::getAvgSDEnergy() const
{
	return avgSdEnergy;
}

double MonitoringData::getCurrentSDPower() const
{
	return currentSdPower;
}

double MonitoringData::getNICEnergy() const
{
	return nicEnergy;
}

double MonitoringData::getAvgNICEnergy() const
{
	return avgNicEnergy;
}

double MonitoringData::getCurrentNICPower() const
{
	return currentNicPower;
}

void MonitoringData::updateCPUEnergy(double energy)
{
	cpuEnergy += energy;
	currentCpuPower = energy;
	avgCpuEnergy += energy;
	avgCpuEnergy /= 2;
}

void MonitoringData::updateGPUEnergy(double energy)
{
	gpuEnergy += energy;
	currentGpuPower = energy;
	avgGpuEnergy += energy;
	avgGpuEnergy /= 2;
}

void MonitoringData::updateSDEnergy(double energy)
{
	sdEnergy += energy;
	currentSdPower = energy;
	avgSdEnergy += energy;
	avgSdEnergy /= 2;
}

void MonitoringData::updateNICEnergy(double energy)
{
	nicEnergy += energy;
	currentNicPower = energy;
	avgNicEnergy += energy;
	avgNicEnergy /= 2;
}

void MonitoringData::addIrp(ULONGLONG irpAddress, const IoEventInfo& info) {
	irpMap[irpAddress] = info;
}

void MonitoringData::updateIrp(ULONGLONG irpAddress, ULONG bytesTransferred) {
	if (irpMap.find(irpAddress) != irpMap.end()) {
		irpMap[irpAddress].bytesTransferred += bytesTransferred;
	}
}

void MonitoringData::removeIrp(ULONGLONG irpAddress) {
	irpMap.erase(irpAddress);
}