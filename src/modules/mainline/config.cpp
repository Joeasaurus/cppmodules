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
	bool runAgain = true;
	while (runAgain) {
		runAgain = this->pollAndProcess();
	}
	return false;
}

bool ConfigModule::process_message(const std::string& message, const std::vector<std::string>& tokens) {
	this->logger->info("{}: {}", this->name(), message);
	return true;
}

ConfigModule* loadModule() {
	return new ConfigModule;
}

void unloadModule(ConfigModule* module) {
	delete module;
}