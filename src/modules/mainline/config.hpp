#pragma once
// Common
#include "main/module.hpp"
// Module Specific
#include <boost/filesystem.hpp>
#include <fstream>

using namespace cppm;

class ConfigModule : public Module {
	public:
		ConfigModule() : Module("config", "Joe Eaves"){};
		~ConfigModule();
		bool run();
		bool process_message(const Message& message, CatchState cought, SocketType sockT);

		bool loadConfigFile(string filepath);
	private:
		Message configOnDisk;
		string configFilepath;
};

// Init/Del functions.
extern "C" ConfigModule* createModule();
extern "C" void destroyModule(ConfigModule* module);
