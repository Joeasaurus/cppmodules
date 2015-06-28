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
	bool runAgain = true;
	while (runAgain) {
		runAgain = this->pollAndProcess();
	}
	return false;
}


bool CoreModule::process_message(const json::value& message, CatchState cought) {
	this->logger->debug("{}: {}", this->name(), stringify(message));
	return true;
}

CoreModule* loadModule() {
	return new CoreModule;
}

void unloadModule(CoreModule* module) {
	delete module;
}