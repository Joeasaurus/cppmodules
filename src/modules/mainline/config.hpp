#pragma once
// Common
#include "main/module.hpp"
#include "Eventer.hpp"
// Module Specific
#include <boost/filesystem.hpp>
#include <fstream>

using namespace cppm;
using namespace cppevent;

class ConfigModule : public Module {
	public:
		ConfigModule() : Module("config", "Joe Eaves"){};
		~ConfigModule();
		void run();
		bool process_message(const Message& message, CatchState cought, SocketType sockT);

		bool loadConfigFile(string filepath);
	private:
		Eventer _eventer;
		Message configOnDisk;
		string configFilepath;
};

// Init/Del functions.
extern "C" ConfigModule* createModule();
extern "C" void destroyModule(ConfigModule* module);
