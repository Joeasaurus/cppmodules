#include "modules/base_module.hpp"

BaseModule::BaseModule() {
	this->__info.name   = "Base";
	this->__info.author = "Joe Eaves";
	std::cout << "[" << this->name() << "] loaded!" << std::endl;
}

BaseModule::~BaseModule() {
	this->closeSockets();
	std::cout << "[" << this->name() << "] Closed!" << std::endl;
}

std::string BaseModule::name() {
	return this->__info.name;
}

void BaseModule::run() {
	while (true) {
		std::string messageText = this->recvMessage(SocketType::PUB);
		std::vector<std::string> messageTokens;
		boost::split(messageTokens, messageText, boost::is_any_of(" "));
		if (messageTokens.at(1) == "close") {
			std::cout << "[" << this->name() << "] Closing..." << std::endl;
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