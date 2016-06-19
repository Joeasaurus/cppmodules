#pragma once
#include "main/module.hpp"
#include "main/messages/messages.hpp"

using namespace cppm;
using namespace cppm::messages;

class OutputModule : public Module {
	public:
		OutputModule() : Module("output", "Joe Eaves"){};
		void setup();
		void tick();
	private:
		Message message{"output"};

		void hookOut();

};

// Init/Del functions.
extern "C" CPPM_WINEXPORT OutputModule* createModule();
extern "C" CPPM_WINEXPORT void destroyModule(OutputModule* module);
