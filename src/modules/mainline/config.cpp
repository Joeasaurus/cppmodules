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
	if (boost::filesystem::is_regular_file(filepath)) {
		try {
			this->config.readFile(filepath.c_str());
			this->logger->debug("{}: {}", this->name(), "Config " + filepath + " loaded!");
			return true;
		} catch(const libconfig::FileIOException &fioex) {
			this->logger->debug("{}: {}", this->name(), "Error: I/O error while reading " + filepath);
			return false;
		} catch(const libconfig::ParseException &pex) {
			this->logger->debug() << "{}: {}" << this->name() << "Error: Parse failed at " <<
				pex.getFile() << ":" << pex.getLine() << " - " << pex.getError();
			return false;
		}
	} else {
		return false;
	}
}

bool ConfigModule::run() {
	bool runAgain = true;
	while (runAgain) {
		runAgain = this->pollAndProcess();
	}
	return false;
}

bool ConfigModule::process_message(const json::value& message, CatchState cought) {
	this->logger->debug("{}: {}", this->name(), stringify(message));
	if (cought == CatchState::FOR_ME) {
		if (json::has_key(message["data"], "command")) {
			if (to_string(message["data"]["command"]) == "load" &&
				json::has_key(message["data"], "file")
			) {
				this->loadConfigFile(to_string(message["data"]["file"]));
			}
		}
	}
	return true;
}

ConfigModule* loadModule() {
	return new ConfigModule;
}

void unloadModule(ConfigModule* module) {
	delete module;
}