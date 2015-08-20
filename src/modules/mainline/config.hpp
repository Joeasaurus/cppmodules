#pragma once
// Common
#include "main/module.hpp"
// Module Specific
#include <boost/filesystem.hpp>
#include <fstream>

class ConfigModule : public Module {
	public:
		ConfigModule() : Module("mainline_config", "Joe Eaves"){};
		~ConfigModule();
		bool run();
		bool process_message(const WireMessage& message, CatchState cought, SocketType sockT);

		bool loadConfigFile(string filepath);
	private:
		WireMessage loadedConfig;
		string configFilepath;
};

// Init/Del functions.
extern "C" ConfigModule* createModule();
extern "C" void destroyModule(ConfigModule* module);
