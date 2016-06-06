#pragma once
// Common
#include "main/module.hpp"

using namespace cppm;

class InterfaceFixture : public Module {
	public:
		InterfaceFixture() : Module("InterfaceFixture", "TestSuite"){};
		~InterfaceFixture(){};
		inline void setup(){};
		inline bool process_command(const Message& message){_logger.null(message.serialise());return true;};
		inline bool process_input  (const Message& message){_logger.null(message.serialise());return true;};
};

// Init/Del functions.
extern "C" CPPM_WINEXPORT InterfaceFixture* createModule();
extern "C" CPPM_WINEXPORT void destroyModule(InterfaceFixture* module);
