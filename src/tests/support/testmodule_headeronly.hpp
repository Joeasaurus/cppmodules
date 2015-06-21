#pragma once
#include "main/module.hpp"

class TestModule : public Module {
	public:
		TestModule() { this->__info.name = "TestModule"; };
		bool run() { while(true){this->pollAndProcess();} return false; };
		bool process_message(const std::string& message, const std::vector<std::string>& tokens){return true;};
};