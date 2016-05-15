#pragma once
// Common
#include "main/module.hpp"

using namespace cppm;

class InterfaceFixture : public Module {
	public:
		InterfaceFixture() : Module("InterfaceFixture", "TestSuite"){};
		~InterfaceFixture(){};
		inline void setup(){};
		inline bool process_command(const Message& message){_logger.null(message.format());return true;};
		inline bool process_input  (const Message& message){_logger.null(message.format());return true;};
};

// Init/Del functions.
extern "C" CPPMAPI InterfaceFixture* createModule();
extern "C" CPPMAPI void destroyModule(InterfaceFixture* module);
