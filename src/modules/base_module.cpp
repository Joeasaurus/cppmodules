#include "modules/base_module.hpp"

BaseModule::BaseModule() {
	this->__info.name   = "Base";
	this->__info.author = "Joe Eaves";
	std::cout << "[" << this->name() << "] loaded!" << std::endl;
}

std::string BaseModule::name() {
	return this->__info.name;
}

void BaseModule::close() {
	this->closeSockets();
	std::cout << "[" << this->name() << "] Closed!" << std::endl;
}

void BaseModule::run() {
	while (true) {
		std::string messageText = this->recvMessage("publish");
		std::vector<std::string> messageTokens;
		this->splitString(messageText, ' ', messageTokens);
		if (messageTokens.at(1) == "close") {
			std::cout << "[" << this->name() << "] Closing..." << std::endl;
			this->close();
			break;
		}
	}
}

BaseModule* loadModule() {
	return new BaseModule;
}

void unloadModule(BaseModule* module) {
	delete module;
}