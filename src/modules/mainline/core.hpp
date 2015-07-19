#pragma once
// Common
#include "main/module.hpp"
// Module Specific

class CoreModule : public Module {
	public:
		CoreModule() : Module("Core", "mainline"){};
		~CoreModule();
		bool run();
		bool process_message(const json::value& message, CatchState cought, SocketType sockT);
};

// Init/Del functions.
extern "C" CoreModule* createModule();
extern "C" void destroyModule(CoreModule* module);
