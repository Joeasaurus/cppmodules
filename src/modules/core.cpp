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

bool CoreModule::run() {
	try {
		while (true) {
			if (this->recvMessage<bool>(SocketType::PUB, [&](const std::string& messageText) {
				if (!messageText.empty()) {
					std::vector<std::string> messageTokens;
					boost::split(messageTokens, messageText, boost::is_any_of(" "));
					if (messageTokens.size() > 0) {
						if (messageTokens.at(1) == "close") {
							std::cout << "[" << this->name() << "] Closing..." << std::endl;
							return true;
						}
					}
				}
				return false;
			}, 500)) {
				break;
			}
		}
		return true;
	} catch(...) {
		return false;
	}
}

CoreModule* loadModule() {
	return new CoreModule;
}

void unloadModule(CoreModule* module) {
	delete module;
}