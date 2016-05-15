#pragma once
// Common
#include "main/module.hpp"
#include "main/messages/messages.hpp"
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
		~ConfigModule();
		void setup();
		void tick();
		bool process_command(const Message& message);
		bool process_input(const Message& message);

		bool loadConfigFile(string filepath);
	private:
		Message configOnDisk;
		string configFilepath = "/home/jme/code/cppmodules/build/modules/main.cfg";
};

// Init/Del functions.
extern "C" CPPMAPI ConfigModule* createModule();
extern "C" CPPMAPI void destroyModule(ConfigModule* module);
