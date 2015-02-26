#include "base_module.hpp"

BaseModule::BaseModule() {
	this->__info.name   = "Base";
	this->__info.author = "Joe Eaves";
	std::cout << this->name() << " loaded!" << std::endl;
}

std::string BaseModule::name() {
	return this->__info.name;
}

void BaseModule::close() {
	std::cout << "Base Closed!" << std::endl;
}

BaseModule* loadModule() {
	return new BaseModule;
}

void unloadModule(BaseModule* module) {
	delete module;
}