#pragma once
// Common
#include "main/module.hpp"
// Module Specific
#include <boost/filesystem.hpp>
#include <fstream>
#include "Eventer.hpp"

using namespace cppm;
using namespace cppm::messages;
using namespace cppevent;

class ConfigModule : public Module {
	public:
		Eventer _eventer;

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
