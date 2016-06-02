#pragma once
#include "main/module.hpp"
#include "main/messages/messages.hpp"

using namespace cppm;
using namespace cppm::messages;

class InputModule : public Module {
	public:
		InputModule() : Module("input", "Joe Eaves"){};
		void setup();
		void tick();
	private:
		Input message{"output"};

};

// Init/Del functions.
extern "C" CPPMAPI InputModule* createModule();
extern "C" CPPMAPI void destroyModule(InputModule* module);
