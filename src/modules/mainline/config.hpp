#pragma once
// Common
#include "main/module.hpp"
// Module Specific
#include <boost/filesystem.hpp>
#include <libconfig.h++>

class ConfigModule : public Module {
	public:
		ConfigModule() : Module("Config", "mainline"){};
		~ConfigModule();
		bool run();
		bool process_message(const json::value& message, CatchState cought, SocketType sockT);

		bool loadConfigFile(string filepath);
	private:
		libconfig::Config config;
		string configFilepath;
};

// Init/Del functions.
extern "C" ConfigModule* createModule();
extern "C" void destroyModule(ConfigModule* module);
