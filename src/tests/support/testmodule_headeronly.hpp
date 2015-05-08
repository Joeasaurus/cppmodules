#pragma once
#include "main/module.hpp"

class TestModule : public Module {
	public:
		TestModule() { this->__info.name = "TestModule"; };
		bool run() { return true; };
};