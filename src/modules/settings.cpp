#include "modules/settings.hpp"

SettingsModule::SettingsModule() {
	this->__info.name   = "Settings";
	this->__info.author = "mainline";
}

SettingsModule::~SettingsModule() {
	this->closeSockets();
	this->logger->info("{}: {}", this->name(), "Closed");
}

bool SettingsModule::loadSettingsFile(std::string filepath) {
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

bool SettingsModule::run() {
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

SettingsModule* loadModule() {
	return new SettingsModule;
}

void unloadModule(SettingsModule* module) {
	delete module;
}