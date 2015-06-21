#pragma once
// Common
#include "main/module.hpp"
// Module Specific

class CoreModule : public Module {
	public:
		CoreModule();
		~CoreModule();
		bool run();
		bool process_message(const std::string& message, const std::vector<std::string>& tokens);
};

// Init/Del functions.
extern "C" CoreModule* loadModule();
extern "C" void unloadModule(CoreModule* module);
