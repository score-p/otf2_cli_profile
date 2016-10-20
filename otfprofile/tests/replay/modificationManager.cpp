#include "modificationManager.h"

ModManager::ModManager(const ModOptions& options) : options(options)
{
}

ModManager::ModManager(const ModManager& manager) : options(manager.options) 
{ 
}

ModManager& ModManager::operator=(const ModManager& manager)
{
	if (this == &manager) return *this;
	options = manager.options;
	return *this;
}

uint32_t ModManager::dsize(uint32_t osize) const
{ 
	return (uint32_t)(osize * this->options.dsize_factor);
}

uint32_t ModManager::wtime(uint32_t wtime) const
{
	return (uint32_t)(wtime * this->options.wtime_factor);
}
