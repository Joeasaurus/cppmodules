#include "base_module.hpp"

BaseModule::BaseModule() {
	this->__info.name   = "BaseModule";
	this->__info.author = "Joe Eaves";
	std::cout << this->name() << " loaded!" << std::endl;
}

BaseModule::~BaseModule() {
	std::cout << "Deconstructed BaseModule!" << std::endl;
}

std::string BaseModule::name() {
	return this->__info.name;
}

BaseModule* loadModule() {
	return new BaseModule;
}

void unloadModule(BaseModule* module) {
	delete module;
}