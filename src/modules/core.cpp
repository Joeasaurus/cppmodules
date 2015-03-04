#include "modules/core.hpp"

CoreModule::CoreModule() {
	this->__info.name   = "Core";
	this->__info.author = "Joe Eaves";
	std::cout << "[" << this->name() << "] loaded!" << std::endl;
}

CoreModule::~CoreModule() {
	this->closeSockets();
	std::cout << "[" << this->name() << "] Closed!" << std::endl;
}

std::string CoreModule::name() {
	return this->__info.name;
}

void CoreModule::run() {
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

CoreModule* loadModule() {
	return new CoreModule;
}

void unloadModule(CoreModule* module) {
	delete module;
}