#include "process.h"

process::process(string pid, string name)
{
	this->setPid(pid);
	this->setName(name);
}

string process::getPid() const
{
	return this->m_pid;
}

string process::getName()
{
	return this->m_name;
}

void process::setPid(string pid)
{
	this->m_pid = pid;
}

void process::setName(string name)
{
	this->m_name = name;
}