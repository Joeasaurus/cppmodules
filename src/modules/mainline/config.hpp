#pragma once
// Common
#include "main/module.hpp"
// Module Specific
#include <boost/filesystem.hpp>
#include <fstream>

using namespace cppm;
using namespace cppm::messages;

class ConfigModule : public Module {
	public:

		ConfigModule() : Module("config", "Joe Eaves"){};
		void setup();
		void tick();

		bool loadConfigFile(string filepath);
	private:
		Message configOnDisk;
		string configFilepath = "/home/jme/code/cppmodules/build/modules/main.cfg";
};

// Init/Del functions.
extern "C" CPPM_WINEXPORT ConfigModule* createModule();
extern "C" CPPM_WINEXPORT void destroyModule(ConfigModule* module);
