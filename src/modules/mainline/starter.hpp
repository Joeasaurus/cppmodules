#pragma once
// Common
#include "main/module.hpp"
// Module Specific

class StarterModule : public Module {
	public:
		StarterModule() : Module("mainline_starter", "Joe Eaves"){};
		~StarterModule();
		bool run();
		bool process_message(const json::value& message, CatchState cought, SocketType sockT);
};

// Init/Del functions.
extern "C" StarterModule* createModule();
extern "C" void destroyModule(StarterModule* module);
