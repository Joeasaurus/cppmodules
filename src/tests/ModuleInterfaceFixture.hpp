#pragma once
#include "main/module.hpp"

using namespace cppm;

class InterfaceFixture : public Module {
	public:
		InterfaceFixture() : Module("InterfaceFixture", "TestSuite"){};
		~InterfaceFixture();
		bool run();
		bool process_message(const Message& message, CatchState cought, SocketType sockT);
};
