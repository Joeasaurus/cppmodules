#pragma once
// Common
#include "main/module.hpp"
// Module Specific
#include <libconfig.h++>

class SettingsModule : public Module {
	public:
		SettingsModule();
		~SettingsModule();
		bool run();

		bool loadSettingsFile(std::string filepath);
	private:
		libconfig::Config config;
};

// Init/Del functions.
extern "C" SettingsModule* loadModule();
extern "C" void unloadModule(SettingsModule* module);
