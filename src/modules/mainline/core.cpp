#include "modules/mainline/core.hpp"

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

bool CoreModule::process_message(const json::value& message, CatchState cought, SocketType sockT) {
	this->logger->debug("{}: {}", this->name(), stringify(message));
	return true;
}

CoreModule* createModule() {return new CoreModule;}
void destroyModule(CoreModule* module) {delete module;}