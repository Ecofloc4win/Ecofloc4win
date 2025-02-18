/**
 * @file process.cpp
 * @brief Definition of the process class
 * @author Ecofloc's Team
 * @date 2025-02-03
 */

#include "process.h"

process::process(std::string pid, std::string name)
{
	this->setPid(pid);
	this->setName(name);
}

std::string process::getPid() const
{
	return this->m_pid;
}

std::string process::getName()
{
	return this->m_name;
}

void process::setPid(std::string pid)
{
	this->m_pid = pid;
}

void process::setName(std::string name)
{
	this->m_name = name;
}