#include "modules/mainline/starter.hpp"

StarterModule::~StarterModule() {
	this->closeSockets();
	this->logger->debug("{}: {}", this->name(), "Closed");
}

bool StarterModule::run() {
	bool runAgain = true;
	while (runAgain) {
		runAgain = this->pollAndProcess();
	}
	return false;
}

bool StarterModule::process_message(const json::value& message, CatchState cought, SocketType sockT) {
	this->logger->debug("{}: {}", this->name(), stringify(message));
	return true;
}

StarterModule* createModule() {return new StarterModule;}
void destroyModule(StarterModule* module) {delete module;}