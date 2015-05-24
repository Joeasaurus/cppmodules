#include "modules/mainline/core.hpp"

CoreModule::CoreModule() {
	this->__info.name   = "Core";
	this->__info.author = "Joe Eaves";
}

CoreModule::~CoreModule() {
	this->closeSockets();
	this->logger->debug("{}: {}", this->name(), "Closed");
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
							return true;
						}
					}
				}
				return false;
			}, 500)) {
				this->logger->debug("{}: {}", this->name(), "Closing...");
				return true;
			}
		}
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