#include "modules/mainline/config.hpp"

ConfigModule::ConfigModule() {
	this->__info.name   = "Config";
	this->__info.author = "mainline";
}

ConfigModule::~ConfigModule() {
	this->closeSockets();
	this->logger->debug("{}: {}", this->name(), "Closed");
}

bool ConfigModule::loadConfigFile(std::string filepath) {
	try {
		this->config.readFile(filepath.c_str());
		return true;
	} catch(const libconfig::FileIOException &fioex) {
		this->logger->debug("{}: {}", this->name(), "Error: I/O error while reading " + filepath);
		return false;
	} catch(const libconfig::ParseException &pex) {
		this->logger->debug() << "{}: {}" << this->name() << "Error: Parse failed at " <<
			pex.getFile() << ":" << pex.getLine() << " - " << pex.getError();
		return false;
	}
}

bool ConfigModule::run() {
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

ConfigModule* loadModule() {
	return new ConfigModule;
}

void unloadModule(ConfigModule* module) {
	delete module;
}