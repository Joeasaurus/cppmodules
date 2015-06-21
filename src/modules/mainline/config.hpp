#pragma once
// Common
#include "main/module.hpp"
// Module Specific
#include <libconfig.h++>

class ConfigModule : public Module {
	public:
		ConfigModule();
		~ConfigModule();
		bool run();
		bool process_message(const std::string& message, const std::vector<std::string>& tokens);

		bool loadConfigFile(std::string filepath);
	private:
		libconfig::Config config;
};

// Init/Del functions.
extern "C" ConfigModule* loadModule();
extern "C" void unloadModule(ConfigModule* module);
