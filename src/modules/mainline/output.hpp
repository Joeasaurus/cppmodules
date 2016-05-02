#pragma once
// Common
#include "main/module.hpp"
#include "Eventer.hpp"

using namespace cppm;
using namespace cppm::messages;
using namespace cppevent;

class OutputModule : public Module {
	public:
		OutputModule() : Module("output", "Joe Eaves"){};
		~OutputModule();
		void setup();
		void tick();
		bool process_command(const Message& message){_logger.null(message.format());return true;};
		bool process_input  (const Message& message){_logger.null(message.format());return true;};
	private:
		Eventer _eventer;
		Output message{"output"};

};

// Init/Del functions.
extern "C" OutputModule* createModule();
extern "C" void destroyModule(OutputModule* module);
